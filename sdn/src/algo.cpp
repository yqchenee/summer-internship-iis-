#include "algo.h"
#include <math.h>
#include <cfloat>
#include <climits>
#include <queue>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std ;

void algo::readGraph( const string& s )
{
    _myGraph.readGraph( s ) ;
}

// check if bn_after and dest_after is same or not
bool algo::is_child_same( const treeNode* tn_before , const treeNode* tn_after ) 
{
    if( same_vector( tn_before-> bn_after , tn_after -> bn_after ) )
    {
        if( same_vector( tn_before -> dest_after , tn_after -> dest_after ))
            return true ;
    }
    return false ;
}

//***************************//
//       CALCULATE COST      //
//***************************//
TotalCost algo::calculate_tree_cost( const Tree& t_before , const Tree& t_after )
{
	if( t_after._segmentTreeInside ) return calculate_segtreeInside_cost( t_before , t_after ) ;
    TotalCost ret ;
    unsigned node_size = t_before._treeNodes.size() ;
    
    for( unsigned i = 0 ; i < node_size ; i++ )
    {
        const treeNode* tn_before = t_before._treeNodes[i] ;
        const treeNode* tn_after = t_after._treeNodes[i] ;
        
        calculate_each_tn_Cost( ret , tn_before , tn_after ) ;
    }
    return ret ;
}

TotalCost algo::calculate_segtreeInside_cost( const Tree& t_before , const Tree& t_after ) 
{
	TotalCost ret ;
    unsigned node_size = _myGraph._maxID + 1 ;

	Key k( t_after._segmentTreeInside ) ;
	// use key to get segmentTree info in hash - _mySeg
    vector< unsigned >& used_aft_tn  = _mySeg.get_used_after( k ) ; 
    vector< unsigned >& used_bef_tn  = _mySeg.get_used_before( k ) ; 
    vector< unsigned >& not_same_tn  = _mySeg.get_same_node( k ) ; // save not same

    for( unsigned i = 0 ; i < node_size ; i++ )
    {
        const treeNode* tn_before = t_before._treeNodes[i] ;
        const treeNode* tn_after = t_after._treeNodes[i] ;

		if( tn_after -> _state >= 5 ) {
			bool used_before = tn_before-> bn_after.size() || tn_before -> dest_after.size() ;
			bool used_after = tn_after-> bn_after.size() || tn_after -> dest_after.size() ;

			if( find( used_aft_tn.begin() , used_aft_tn.end() , i ) == used_aft_tn.end() ) { // not find
				if( tn_after -> _state == SubTreeNormal || tn_after -> _state == SubTreeSource ) { 
					if( used_after ) {
						used_aft_tn.push_back( i ) ;
					}
					if( used_before ) {
						used_bef_tn.push_back( i ) ;
					}
				}
				else { // branch leaves 
					used_aft_tn.push_back( i ) ;
					used_bef_tn.push_back( i ) ;
				}
			}
			if( find( not_same_tn.begin() , not_same_tn.end() , i ) == not_same_tn.end() ) {
				if( used_after && used_before ) {
					if( !is_child_same( tn_before , tn_after ) ) { 
						not_same_tn.push_back( i ) ;
					} 
				}
			}
			continue ;
		}
		else {
			bool segConnected = ( tn_after -> _state >= 3 ) ? 1 : 0 ;
			calculate_each_tn_Cost( ret , tn_before , tn_after , segConnected ) ;
		}
    }
    return ret ;
}

void algo::calculate_each_tn_Cost( TotalCost& ret , const treeNode* tn_before , const treeNode* tn_after , bool segConnected ) 
{
	// node is branch state node or not
	unsigned used_before = tn_before-> bn_after.size() + tn_before -> dest_after.size() ;
	unsigned used_after = tn_after-> bn_after.size() + tn_after -> dest_after.size() ;
	
	if( used_before + used_after == 0 ) return ;
	assert( (used_after + used_before) >= 2 || 	
				tn_after -> treeNode_state() == Source ) ;
	
	used_before = ( used_before >= 2 ) ? 1 : 0 ;
	used_after = ( used_after >= 2 ) ? 1 : 0 ;
	bool is_same = is_child_same( tn_before , tn_after ) ;

	unsigned TRI =  used_before * used_after * (unsigned)is_same ; // TRI is 0 or 1
	
	if( tn_before -> _state < 5 )
		ret._rule += used_before + used_after - TRI ;

	if( segConnected ) {
		ret._seg_update 	+=  (used_after  - TRI) / RHO ;
		ret._seg_transition +=  (used_before * used_after - TRI) / TAU ;
	}
	else {
		ret._updateCost 	+=  (used_after  - TRI)  ;
		ret._transitionCost +=  (used_before * used_after - TRI)  ;
	}
}

// all segmentTree calculate cost
TotalCost algo::segTree_update_transition_Cost() 
{
	TotalCost ret ;
	bool* is_check = new bool [ SOURCE_NUM ] ;
	for( unsigned i = 0 ; i < SOURCE_NUM ; i++ ) is_check[i] = 0 ;

	for( unsigned tree_id = 0 ; tree_id < SOURCE_NUM ; tree_id++ )
	{
		if( is_check[ tree_id ] ) continue ;
		if( _trees[ tree_id ]-> _tree ._segmentTreeInside )
		{
			Key k( _trees[ tree_id ]-> _tree ._segmentTreeInside ) ;
    		vector< unsigned >& used_aft_tn  = _mySeg.get_used_after( k ) ; 
    		vector< unsigned >& used_bef_tn  = _mySeg.get_used_before( k ) ; 
    		vector< unsigned >& not_same_tn  = _mySeg.get_same_node( k ) ; 
    		vector< unsigned >& tree_used    = _mySeg.get_tree_used( k ) ; 
			unsigned after_s = used_aft_tn.size() ;
			unsigned before_s = used_bef_tn.size() ;
			if( before_s > after_s ) before_s = after_s ;

			ret._rule 			+= ( not_same_tn.size() ) ;
			ret._seg_update 	+= ( after_s - before_s + not_same_tn.size()) / RHO ;
			ret._seg_transition += not_same_tn.size() / TAU ;
			used_aft_tn.clear() ;
			used_bef_tn.clear() ;
			not_same_tn.clear() ;

			for( unsigned i = 0 ; i < tree_used.size() ; i++ )
			{
				assert( _trees[ tree_used[i] ]-> _tree ._segmentTreeInside ) ;
				is_check[ tree_used[i] ] = 1 ;
			}
		}
	}
	return ret ;
}

//***************************//
//       JOIN AND LEAVE      //
//***************************//
void algo::join( const vector < int >& dsts, int tree_index )
{
	Tree& tree = _trees[tree_index]-> _tree ;
    for( unsigned i = 0 ; i < dsts.size() ; i++ )
	{
		tree.insert_NewDest( dsts[i] ) ;
    }
}

void algo::join_BBSRT (const vector < int >& dsts, int tree_index )
{
	Tree& tree = _trees[tree_index]-> _tree ;
    for( unsigned i = 0 ; i < dsts.size() ; i++ )
	{
		if( tree._treeNodes[ dsts[i]] -> _state == Destination )
		{
			tree.insert_NewDest_BBSRT( dsts[i] ) ;
			continue ;
		}
		_myGraph.shortest_path_proc( dsts[i] ) ;

		// find max_branch_node
		unsigned max_branch = 0 ;
		vector<unsigned> max_b_id ;
		for( unsigned id = 0 ; id <= _myGraph._maxID ; id++ )
		{
			if( tree._treeNodes[ id ] -> _state == Source ) continue ;
			if( tree._treeNodes[ id ] -> _use_num ) 
			{
				unsigned size = tree.get_usedChild( id ).size() ;
				if( size > max_branch ) {
					max_branch = size ;
					max_b_id.clear() ;
					max_b_id.push_back( id ) ;
				}
				else if ( size == max_branch ) {
					max_b_id .push_back( id ) ;
				}
			}
		}

		// find minimum num_hop
		vector< unsigned > path ;
		unsigned num_hop = UINT_MAX ;
		unsigned nearst_id = UINT_MAX ;
		
		for( unsigned j = 0 ; j < max_b_id.size() ; j++ )
		{
			path.clear() ;
			_myGraph.getPath( path , max_b_id[j] ) ;
			if( path.size() < num_hop ) {
				if( find( path.begin() , path.end() , tree._source ) == path.end() ) { // not find
					for( unsigned j = 0 ; j < path.size() ; j++ ) {
						unsigned k = j+1 ;
						if( tree._treeNodes[ path[j] ] -> _parent == path[k] ) {
							path.clear() ;
							break ;
						}
						if( k == ( path.size() -1 ) ) break ;
					}
					if( path.size() ) {
						num_hop = path.size() ;
						nearst_id = max_b_id[j] ;
					}
				}
			}
		}
		
		if( num_hop != UINT_MAX ) {
			path.clear() ;
			_myGraph.getPath( path , nearst_id ) ;
			// rerout by path
			for( unsigned j = 0 ; j < path.size() ; j++ ) {
				unsigned k = j+1 ;
				if( k == ( path.size() ) ) break ;
				tree.change_parent( path[ k ] , path[ j ] ) ;
			}
		}
		tree.insert_NewDest_BBSRT( dsts[i] ) ;
		tree.renew_tree_usage() ;
    }
}

void algo::leave( const vector < int >& dsts, int tree_index )
{
    Tree& tree = _trees[tree_index]-> _tree ;
	for( unsigned i = 0 ; i < dsts.size() ; i++)
	{
		tree.delete_Dest( dsts[i] ) ;
	}
}

//***************************//
//       SEGMENTTREE         //
//***************************//
// for tree a and tree b , find it common part( segment tree )
subTree* algo::find_segmenttree( const unsigned& a , const unsigned& b )
{
    const Tree& trA = _trees[a]-> _tree ;
	const Tree& trB = _trees[b]-> _tree ;
	if( trA._segmentTreeInside || trB._segmentTreeInside ) return NULL ;
	
	trA.init_treeNode() ;  // walk through treeA 

	queue< unsigned > id_q ; // bfs
	id_q.push( trA._source );

	while( !id_q.empty() )
	{
		unsigned node_id = id_q.front() ;
		id_q.pop();
		treeNode* tempA = trA._treeNodes[ node_id ] ;
		treeNode* tempB = trB._treeNodes[ node_id ] ;

		if( tempA -> _is_visit ) continue ; 

		const vector< unsigned >& node_child_A = trA.get_usedChild( node_id ) ; 
		for( unsigned i = 0 ; i < node_child_A.size() ; i++ )
		{
			id_q.push( node_child_A[i] ); 
		}
		
        if( tempA -> _use_num && tempB -> _use_num )
		{
            // node_id is source
			treeNode* tn = new treeNode( tempA , "subtree" ) ;
			Tree* newTree = new Tree( tn , _myGraph._maxID ) ; 
			unsigned valid_tree = 1 ;

			const vector< unsigned >& node_child_B = trB.get_usedChild( node_id ) ;
			
			// after finding source , use bfs to find segTree child
			// find A and B same childs after segT source
			vector< unsigned > common_child = find_common( node_child_A , node_child_B ) ;
			while( !common_child.empty())
			{
				unsigned child_id = common_child.back() ;
				common_child.pop_back() ;
				treeNode* childA = trA._treeNodes[ child_id ] ;
				treeNode* childB = trB._treeNodes[ child_id ] ; 	
				
				if( childA -> _use_num && childB-> _use_num ) 
				{
					childA -> _is_visit = 1 ;
					treeNode* tn2 = new treeNode( childA , "subtree" ) ; 
					newTree-> _treeNodes[ child_id ] = tn2 ; // id of tn2
					valid_tree ++ ;

					// update parent's child
					treeNode* parent = newTree -> _treeNodes[ tn2-> _parent ] ;
					parent -> new_child( child_id ) ;

					// find A and B same childs after same childs
					vector<unsigned> v = find_common( childA -> _child , childB -> _child ) ;
					append_vector( common_child , v ) ;
				}
			}
			if( valid_tree >= MIN_SEGMENTREE_SIZE ) {
				subTree* t = new subTree( newTree ) ; 
                return t ;
			}
			else delete newTree ;
		}
		tempA-> _is_visit = 1 ;
	}
    return NULL ;
}

// check tree_id can use sT or not 
void algo::segmentTree_Pairing( const unsigned& segT_ID , const subTree* const sT , 
                        const unsigned& st_o1, const unsigned& st_o2 , const unsigned& tree_id ) 
{
    if( tree_id == st_o1 || tree_id == st_o2 ) return ;
	if( _trees[ tree_id ]-> _tree._segmentTreeInside ) return ;

	const unsigned sT_root = sT-> _tree._source ;

	// from sT root , find head_path to tree
    pair< vector<unsigned> , double > path_and_cost = find_min_path_and_Cost( sT_root , tree_id ) ;
	if( path_and_cost.second == UINT_MAX ) return ;
 
    const vector<unsigned> & 	to_head_path = path_and_cost.first ;
	const unsigned& 			tree_head = to_head_path[0] ;
	
	queue<unsigned> 			tree_child_node = _trees[tree_id]-> _tree. used_child( tree_head ) ;
	double 	origin_Cost = _trees[tree_id]-> _tree.get_treeNode_cost( tree_head ) ; // head cost 

	if( check_rerouting( origin_Cost , tree_child_node , sT , tree_id ) ) // tree_id has enough child to connect to subtree 
	{
		assert( !tree_child_node.empty() ) ;
		vector<unsigned> need_connect_node = queue_to_vector( tree_child_node ) ;
		vector< vector<unsigned >> all_path ;
		if( check_threshold( all_path , origin_Cost , need_connect_node , path_and_cost.second , sT , tree_id ) )
		{
			if( rerouting( all_path , to_head_path , sT , tree_id )) {  // rerout sucess
				_trees[ tree_id ] -> _tree ._segmentTreeInside = segT_ID ;
				Key key( segT_ID ) ;
				_mySeg.push( key , tree_id ) ;
				
				if( segtifyTree( segT_ID , sT , st_o1 , st_o2 ) ) { // if st_o1 , st_o2 is segtify or not
					_mySeg.push( key , st_o1 ) ;
					_mySeg.push( key , st_o2 ) ;
				}
			}
		}
	}
}

// actual rerouting
bool algo::rerouting ( const vector< vector<unsigned >>& all_path , const vector<unsigned>& to_head_path , 
                            const subTree* const sT , const unsigned& tree_id )
{
	if( ! check_path( all_path , to_head_path , sT ) ) return false ; // path not include sT tree node
	cout << "check ok... " ; 
	
	Tree& 					this_Tree = ( _trees[tree_id] -> _tree ) ;
	Tree*               	orig_Tree = new Tree( this_Tree , true ) ;
	vector<treeNode*>&  	tTree_tns = this_Tree.  _treeNodes ;
	unsigned 				tree_head = to_head_path[0] ;
	unsigned				th_usenum = this_Tree.  _treeNodes[ tree_head ] -> _use_num ;
	vector< unsigned >  tree_node_id ;
	tree_node_id.clear() ;

	this_Tree. init_treeNode() ; // all node is_visit = 0 
	for( unsigned i = 0 ; i < all_path.size() ; i++ )
	{
		tree_node_id.push_back( all_path[i][0] ) ;
		this_Tree.record_visited_to_treeHead( tree_node_id[i] , tree_head ) ; 
	}

	// head path 
	if( !this_Tree. connect_path( tree_head , to_head_path , 1 )) 
	{
		cout << "fault" << '\n' ;
		this_Tree = *orig_Tree ;
		return false ;
	}
	
	// segment tree part connect 
	const Tree& segT = sT -> _tree ;
	assert( to_head_path[ to_head_path.size() -1 ] == segT._source ) ;
	assert( segT._source != this_Tree._source ) ;
	this_Tree. segtify_Node( segT._source , segT._treeNodes ) ;

	// all path
	for( unsigned i = 0 ; i < all_path.size() ; i++ )
	{
		if( !this_Tree.connect_path( tree_node_id[i] , all_path[i] ) )
		{
			cout << "fault" << '\n' ;
			this_Tree = *orig_Tree ;
			return false ;
		}
	}
	
	// check tree => loop
	if( !this_Tree.checkTree() ) {
		cout << "fault" << '\n' ;
		this_Tree = *orig_Tree ;
		return false ;
	} 
	
	this_Tree. renew_tree_usage() ;
	cout << "finish rerouting " << tree_id << '\n';

	return true ;
}


// from minCost_matrix , find the good choice
vector< vector<unsigned >> minCostMatch( double& new_cost , pair< vector<unsigned> , double >** new_path_cost , 
					unsigned subtree_leaves_size ,unsigned tree_node_size )
{
	// new_path_cost : matrix[ subtreeNode * treeNode ] 
	assert( subtree_leaves_size <= tree_node_size ) ;
	vector< vector<unsigned >> ret_path ;

	bool* connected = new bool [ tree_node_size ];
	for( unsigned i = 0 ; i < tree_node_size ; i++ ) connected[i] = 0 ;
	
	// first , find  the smallest one in the matrix , record it and mark its column and row
	double rec = DBL_MAX ;
	unsigned id_i , id_j ;
	for( unsigned i = 0 ; i < subtree_leaves_size ; i++ )  // find minimum
	{
		for( unsigned j = 0 ; j < tree_node_size ; j++ ) {
			if( new_path_cost[i][j].second < rec ) {
				rec = new_path_cost[i][j].second ;
				id_i = i ;
				id_j = j ;
		} }
	}
	new_cost += rec ;
	ret_path.push_back( new_path_cost[id_i][id_j].first ) ;
	connected[id_j] = 1 ;
	// second , from each row , find the minimun one , and mark its column
	for( unsigned i = 0 ; i < subtree_leaves_size ; i++ ) 
	{
		if( i == id_i ) continue ;
		rec = DBL_MAX ;
		unsigned id ;
		for( unsigned j = 0 ; j < tree_node_size ; j++ )
		{
			if( connected[j] ) continue ;
			if( new_path_cost[i][j].second < rec ){
				rec = new_path_cost[i][j].second ;
				id = j ;
			}
		}
		new_cost += rec ;
		ret_path.push_back( new_path_cost[i][id].first ) ;
		connected[id] = 1 ;
	}

	// third , for each col which is not connect => find the minimum one
	for( unsigned j = 0 ; j < tree_node_size ; j++ ) // not connect 
	{
		if( connected[j] == 0 ) {
			double rec = DBL_MAX ;
			for( unsigned i = 0 ; i < subtree_leaves_size ; i++ )
			{
				if( new_path_cost[i][j].second < rec ){
					rec = new_path_cost[i][j].second ;
				}
			}
			assert( rec != DBL_MAX ) ;
			new_cost += rec ;
		}
	}
	return ret_path ;
}
// compare with threshold to decide if it could replace by subtree
bool algo::check_threshold( vector< vector<unsigned >>& all_path , 
				const double& origin_Cost , const vector<unsigned>& need_connect_node ,	
				const double& to_head_Cost , const subTree* const sT  , const unsigned& tree_id  )
{
	assert( origin_Cost > 0 ) ;

	const vector<unsigned>& subtree_leaves = sT-> _leaves ;
	const unsigned subtree_leaves_size = subtree_leaves.size();
	const unsigned tree_node_size = need_connect_node.size() ;

	pair< vector<unsigned> , double >** new_path_cost ;
	constructCostMatrix( new_path_cost ,tree_id , subtree_leaves , need_connect_node ) ;
	
	double new_cost = 0;
	
	// subtree leaves to tree node
	all_path = minCostMatch( new_cost , new_path_cost , subtree_leaves_size , tree_node_size ) ;
	new_cost += to_head_Cost ; 			// subtree root to tree head
	new_cost += sT -> _subtree_cost ; 	// subtree inside cost
	
	if( new_cost < origin_Cost * (THRESHOLD_RATIO / _trees[tree_id]-> _tree._flow) ) 
	{
		return true ;
	}
	return false ;
}

// minimum path from root to tree[tree_id]
pair< vector<unsigned> , double > algo::find_min_path_and_Cost ( const unsigned& root , const unsigned& tree_id ) 
{
	_myGraph.shortest_path_proc( root ) ;

    const vector<treeNode* >& thisTree_tns = _trees[ tree_id ]-> _tree._treeNodes ;
	
	unsigned min_id = UINT_MAX ;
	double minCost = DBL_MAX ;

	for( unsigned node_id = 0 ; node_id < thisTree_tns.size() ; node_id++ )
	{
		if( thisTree_tns[node_id]-> _use_num == 0 ) continue ;
		if( thisTree_tns[node_id]-> _state == Destination ) continue ;
		if( _myGraph._nodelist[node_id]-> _cost < minCost ) 
		{
			minCost = _myGraph._nodelist[node_id] -> _cost ;
			min_id = node_id ;
		}
	}

    pair< vector<unsigned> , double > ret ;
	ret.second = minCost ;
	if( minCost == DBL_MAX ) return ret ;

	_myGraph.getPath( ret.first , min_id ) ;

    return ret ;
}

// if all path , to_head_path contains sT's node return false
bool algo::check_path( const vector< vector<unsigned >>& all_path , const vector<unsigned>& to_head_path , 
                            const subTree* const sT )
{
	for( unsigned i = 0 ; i < all_path.size() ; i++ ) {
		for( unsigned j = 0 ; j < all_path[i].size() ; j++ ) {
			unsigned path_node_id = all_path[i][j] ;
			if( sT-> _tree._treeNodes[path_node_id] != NULL ) {
				if( j != all_path[i].size() -1 ) return false ;
			}
			//cout << path_node_id << ' ' ;
		}
		//cout << '\n';
	}
	for( unsigned i = 0 ; i < to_head_path.size() ; i++ ) {
		if( sT-> _tree._treeNodes[ to_head_path[i] ] != NULL ){
			if( i != to_head_path.size() -1 ) return false;
		}
		//cout << to_head_path[i] << ' ' ;
	}
	//cout << endl ;
	return true ;
}

// check if there is enough child to connect to subtree or not 
bool algo::check_rerouting( double& origin_Cost , queue<unsigned>& tree_child_node , 
							const subTree* const sT , const unsigned& tree_id )
{
	const vector<unsigned>& subtree_leaves = sT-> _leaves ;
	const unsigned subtree_leaves_size = subtree_leaves.size();
	
	const Tree& this_Tree = _trees[ tree_id ]-> _tree ;
	for( unsigned i = 0 ; i <= _myGraph._maxID ; i++ )
	{
		if( sT -> _tree._treeNodes[i] == NULL ) continue ;
		if( this_Tree._treeNodes[i] -> _use_num ) return false ;
	}
	
	unsigned tree_node_size = tree_child_node.size() ;

	// tree_node_size has to be larger than subtree_leaves_size
	// bfs to find tree_child_node from tree_head 
	int k = 0 ;
	while( !tree_child_node.empty() && tree_node_size < subtree_leaves_size )
	{
		k++ ;
		unsigned new_node_id = tree_child_node.front() ;
		queue<unsigned> new_child = this_Tree.used_child( new_node_id ) ;

		tree_child_node.pop() ;
		
		if( !new_child.empty() ){
			append_queue( tree_child_node , new_child ) ;
		}
		else{
			tree_child_node.push( new_node_id ) ;
		}

		tree_node_size =  tree_child_node.size() ;
		if( k == 15 ) return false ;
	}
	if( tree_node_size == 0 ) return false ;

	// calculate threshold
	origin_Cost = ( 0 - origin_Cost * tree_node_size ) ; 

	for( unsigned i = 0 ; i < tree_node_size ; i++ )
	{	
		unsigned id = tree_child_node.front() ;
		tree_child_node.pop() ;
		origin_Cost += this_Tree.get_treeNode_cost( id ) ;
		tree_child_node.push( id ) ;
	}
	assert( origin_Cost >= 0 ) ;
	return true ;
}


// from leaves to need connected node , construct matrix to record each leaves to each node path and cost 
void algo::constructCostMatrix( pair< vector<unsigned> , double >**& new_path_cost , const unsigned& tree_id ,\
							const vector<unsigned>& subtree_leaves , const vector<unsigned>& need_connect_node )
{
	const unsigned subtree_leaves_size = subtree_leaves.size();
	const unsigned tree_node_size = need_connect_node.size() ;

	new_path_cost = new pair< vector<unsigned> , double >* [ subtree_leaves_size ] ;

	for( unsigned i = 0 ; i < subtree_leaves_size ; i++ )
	{
		_myGraph.shortest_path_proc( subtree_leaves[i] ) ;
		
		new_path_cost[i] = new pair< vector<unsigned> , double > [ tree_node_size ] ;
		for( unsigned j = 0 ; j < tree_node_size ; j++ )
		{
			new_path_cost[i][j]. second = _myGraph._nodelist[ need_connect_node[j] ] -> _cost ;

			_myGraph.getPath( new_path_cost[i][j]. first , need_connect_node[j] ) ;
		}
	}
}

// use sT treeNode to segtify origin_tree 1 and 2
bool algo::segtifyTree( const unsigned& segT_ID , const subTree* const sT , 
							const unsigned& st_o1 , const unsigned& st_o2 )
{
	unsigned node_size = _myGraph._maxID +1 ;
	const Tree& segT = sT -> _tree ;
	// segT.printTree_topo( 0 , 1 ) ;
	Tree& treeA = _trees[ st_o1 ] -> _tree ;
	Tree& treeB = _trees[ st_o2 ] -> _tree ;
	if( treeA. _segmentTreeInside == segT_ID ) {
		assert( treeB. _segmentTreeInside == segT_ID ) ;
		return false ;
	}
	treeA._segmentTreeInside = segT_ID ;
	treeB._segmentTreeInside = segT_ID ;

	for( unsigned i = 0 ; i < node_size ; i++ ) 
	{
		treeNode* sT_tn = segT._treeNodes[i] ;
		if( sT_tn != NULL ) {
			treeA._treeNodes[i] -> seg_Node( sT_tn ) ;
			treeB._treeNodes[i] -> seg_Node( sT_tn ) ;
			if( sT_tn -> _child.empty() ) { // leaves
				treeA.change_child_State_to_SubtreeConnected( i ) ;
				treeB.change_child_State_to_SubtreeConnected( i ) ;
			}
		}
	}
	return true ;
}

// in checking that whether subtree is still being used or not
bool algo::check_if_subtree_remain( const unsigned& tree_index ) 
{
	Tree& tree = _trees[ tree_index ] -> _tree ;
	return tree. check_subtree_path() ;
}

// after rerout , if subtree is not used , rebuild it by shortest_path_tree
void algo::rebuild_tree( const unsigned& tree_index ) 
{
	Tree& this_tree = _trees[ tree_index ] -> _tree ;
	// source cost parent child 
	Tree shortest_path_tree = _myGraph.build_shortest_path_tree( this_tree._source ) ;
	shortest_path_tree._flow = this_tree._flow ;
	shortest_path_tree._segmentTreeInside = 0 ;

	const vector< treeNode* >& ttr_tns = this_tree._treeNodes ;
	const vector< treeNode* >& spt_tns = shortest_path_tree._treeNodes ;
	for( unsigned i = 0 ; i < this_tree._treeNodes.size() ; i++ )
	{
		spt_tns[i] -> _num_dest = ttr_tns[i] -> _num_dest ;
		spt_tns[i] -> _state    = ( ttr_tns[i] -> _num_dest ) ? Destination : Normal ;
		spt_tns[i] -> bn_after  = ttr_tns[i] -> bn_after ;
		spt_tns[i] -> dest_after = ttr_tns[i] -> dest_after ;
	}
	spt_tns[ this_tree._source ] -> _state = Source ;
	assert( spt_tns[ this_tree._source ] -> _num_dest == 0 ) ;

	this_tree = shortest_path_tree ;
	this_tree.renew_tree_usage() ; // use num
}

void algo::init_pairing( const subTree* const sT )
{
    _myGraph.init_node() ;
    const vector< treeNode* >& sT_tns = sT-> _tree. _treeNodes ;
    for( unsigned i = 0 ; i < sT_tns.size() ; i++ )
    {
        if( sT_tns[i] != NULL )
        {
            _myGraph._nodelist[i]-> _is_visit = 1 ;
        }
    }
}

void algo::print_time_slots(){
    cout << "======================" << '\n' ;
    cout << "PRINT TIME SLOT  ";
    for( int i = 0 ; i < _trees.size() ; i++){
        if( i == 0 ){
            for( int j = 0 ; j < _trees[i]-> _timeSlots.size() ; j++){    
                cout << "trees : " << i << " root : " << _trees[i]-> source << endl;
                cout << "time slot : " << j << " => " << endl  ;
                vector<int> tjoin = _trees[i]-> _timeSlots[j].join ;
                vector<int> tleave = _trees[i]-> _timeSlots[j].leave ;
                cout << "join : " ;
                for( int k = 0 ; k < tjoin.size() ; k++)
                    cout << tjoin[k] << ' '  ;
                cout <<'\n' <<  "leave : " ;
                for( int k = 0 ; k < tleave.size() ; k++)
                    cout << tleave[k] << ' ' ;
                cout << '\n'<< endl;
            }
        }
    }
    cout << "=========================" << endl;
}