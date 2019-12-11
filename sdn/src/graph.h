#include <iostream>
#include <climits>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <stdlib.h>
#include <time.h> 
#include <cassert>
#include "define.h"
using namespace std;

class Node ;

// cost format
class TotalCost{
public:
	TotalCost() : _edgeCost(0) , _transitionCost(0) , _seg_transition(0) ,
				_updateCost(0) , _seg_update(0) , _rule(0) {}
	double _edgeCost ;
	double _transitionCost ;
	double _seg_transition ;
	double _updateCost ;
	double _seg_update ;
	double _rule ;
    
	TotalCost& operator+= ( const TotalCost& tc ){
		_edgeCost += tc._edgeCost ;
		_transitionCost += tc._transitionCost ;
		_seg_transition += tc._seg_transition ;
		_updateCost += tc._updateCost ;
		_seg_update += tc._seg_update ;
		_rule += tc._rule ;	
		return *this ;
	}

	friend ostream& operator<< (ostream& os , const TotalCost& tc ){
		return( os << "edge : "<< tc._edgeCost <<'\n'  
					<< "update : "<< tc._updateCost<< '\n'  
					<< "segment update : " << tc._seg_update << '\n'
					<< "transition : "<< tc._transitionCost << '\n' 
					<< "segment transition : "<< tc._seg_transition << '\n' 
					<< "rule : " << tc._rule << endl ) ;
	}
};
 
class Edge{
public:
	Edge( unsigned na , unsigned nb , double c ) :
		_node1( na ) , _node2( nb ) , _cost( c ) {}
	const unsigned& otherside( unsigned caller ) { // if e( a , b ) , use e.otherside( a ) => get b
		return (( caller == _node1 ) ? _node2 : _node1 );
	}
	const double& getCost(){ return _cost ; }
private:
	unsigned _node1 ;
	unsigned _node2 ;
	double   _cost  ;
};

class Node{
public:
	friend class GraphMgr ;
	friend class algo ;
	friend class treeNode ;
	Node( unsigned id ) : _is_visit(0) , _id( id ) { 
		_outEdges.clear(); 
	}
	void new_Edge( Edge* e ) { _outEdges.push_back( e ) ; }
	
private:
	bool     	_is_visit ; // shortest path algo
	unsigned 	_id ;
	unsigned 	_parent ; // shortest path
	double   	_cost ;   // shortest path proc record every node cost
	vector< Edge* > _outEdges ;
};

class treeNode{
public:
	friend class Tree ;
	friend class algo ;
	friend class subTree ;
	treeNode(){}
	treeNode( Node* n ){  // use n to initialize it => in graphMgr::readGraph
		_id 			= n -> _id ;
		_parent     = n -> _parent  ;
		_cost 		= n -> _cost ;
		_use_num 	= 0 ;
		_num_dest 	= 0 ;
		_state 		= Normal ;
		clear_node() ;
	}
	treeNode( const treeNode* const tn ){ // copy tree to calculate tree cost => in main
		_state = tn -> _state ;
		bn_after = tn -> bn_after ;
		dest_after = tn -> dest_after ;
	}
   treeNode( const bool& b , const treeNode* const tn ){  // copy tree in rerouting => copy everything
		assert( b ) ;
		_id 			= tn -> _id ;
		_parent 		= tn -> _parent ;
		_use_num 	= tn -> _use_num ;
		_num_dest 	= tn -> _num_dest ;
		_state 		= tn -> _state ;
		_cost     	= tn -> _cost ;
		_child      = tn -> _child ;
		bn_after    = tn -> bn_after ;
		dest_after  = tn -> dest_after ;
 	}
	treeNode( const treeNode* const tn , const string& s ){  // construct subtree , child is added from find_subtree()
		assert( s == "subtree") ;
		_is_visit = 0 ;
		_id = tn -> _id ;
		_parent = tn -> _parent ;
		_use_num = tn -> _use_num ;
		_num_dest = tn -> _num_dest ;
		_state = SubTreeNormal ;
		_cost = tn-> _cost ;
 	}

	void clear_node() const { 
		bn_after.clear() ;
		dest_after.clear() ;
	}	
	void remove_child( unsigned child , bool uncertain = 0 ) {  // uncertain = 1 => don't know child is exist or not 
		for( vector<unsigned>::iterator it = _child.begin() ; it != _child.end() ; it++)
		{
			if( *it == child ) { _child.erase( it ) ; return ; }
		} assert( 0 || uncertain ) ;
	}

	void new_child( unsigned child ) { _child.push_back( child ) ; }
	void add_child( unsigned child ) {  // not to add the same child
		for( vector<unsigned>::iterator it = _child.begin() ; it != _child.end() ; it++) {
			if( *it == child )  return ; 
		} 
		new_child( child ) ; 
 	}
	 
	void seg_Node( treeNode* sT_tn ) {  // change the origin tree to have segment tree in it
		assert( sT_tn -> _state >= 5 ) ;
		assert( _parent == sT_tn-> _parent || sT_tn -> _state == SubTreeSource ) ;
		assert( _id == sT_tn -> _id ) ;
		if( _state == Source ) {
			assert( sT_tn -> _state == SubTreeSource ) ;
			return ;
		}
		_state = sT_tn -> _state ;
	}
	treeNode_State& get_treeNode_state() { return _state ;}
	const treeNode_State& treeNode_state() const { return _state ;}
private:
	bool 	            	_is_visit ; // in find subtree ;
	unsigned	        		_id ;
	unsigned 	        	_parent ;
	unsigned            	_use_num ;
	unsigned            	_num_dest ;
	treeNode_State 	   _state;
	double 	            _cost ;
	vector< unsigned >   _child ;
	mutable vector< unsigned > 	bn_after , dest_after ; // calculate update transition 
};

class Tree{
public:
	friend class GraphMgr ;
	friend class algo ;
	friend class subTree ;
	Tree(){ 
		_segmentTreeInside = 0 ;
		_flow = double( double((rand() % (FLOW_SIZE*9) )+10) / 10.0 ) ; 
		// _flow = 1 ;
	}
	Tree( const Tree& t ){ // copy tree in main
		_source = t._source ;
		_flow   = t._flow ;
		for( unsigned i = 0 ; i < t. _treeNodes.size() ; i++ ) {
			treeNode* tn = new treeNode( t._treeNodes[i] ) ;
			_treeNodes.push_back( tn ) ;
		}
	}
	Tree( const Tree& t , const bool& b ) { // copy tree in rerouting
		assert( b ) ;
		_segmentTreeInside = t._segmentTreeInside ;
		_source = t._source ;
		_flow   = t._flow ;
		for( unsigned i = 0 ; i < t. _treeNodes.size() ; i++ ) {
			treeNode* tn = new treeNode( 1 , t._treeNodes[i]) ;
			_treeNodes.push_back( tn ) ;
		}
	}
	Tree( treeNode* tn , unsigned size ) { // newTree => use to construct subtree 
		_treeNodes.resize( size+1 , NULL ) ;
		tn -> _parent = UINT_MAX ; // root 
		tn -> _state = SubTreeSource ;
		_treeNodes[ tn-> _id ] = tn ;
		_source = tn -> _id ;
	}

	// initiation
	void init_treeNode_usenum() {  // in Tree::renew_tree_usage
		for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
			_treeNodes[i] -> _use_num = 0 ;
		}
	}
	void init_treeNode() const {  // is_visit is used in shortest_path_tree_proc , find_segmentTree
		for( unsigned i = 0 ; i < _treeNodes.size() ; i++) {
			_treeNodes[i]-> _is_visit = 0 ;
		}
	}
	void clear_tree_node() const {  // clear bn , dest_after
		for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
			_treeNodes[i]-> clear_node() ;
		}
	}

	vector< unsigned > get_usedChild( unsigned id ) const { // return vector that the treeNode's used_child
		vector< unsigned > ret ;
		ret.clear() ;
		treeNode* tn = _treeNodes[ id ] ;
		assert( tn -> _use_num ) ;
		for( unsigned i = 0 ; i < tn -> _child.size() ; i++)
		{
			unsigned child_id = tn -> _child[i] ;
			if( _treeNodes[ child_id ] -> _use_num ) ret.push_back( child_id ) ;
		}
		return ret ;
	}
	queue<unsigned> used_child( unsigned id ) const { // return queue that the treeNode's used_child
		queue< unsigned > ret ;
		treeNode* tn = _treeNodes[ id ] ;
		assert( tn -> _use_num ) ;
		for( unsigned i = 0 ; i < tn -> _child.size() ; i++)
		{
			unsigned child_id = tn -> _child[i] ;
			if( _treeNodes[ child_id ] -> _use_num ) ret.push( child_id ) ;
		}
		return ret ;
	}
	double get_treeNode_cost(const unsigned& id) const {
		assert( _treeNodes[id]!= NULL ) ;
		return _treeNodes[ id ] -> _cost ;
	}

	void remove_usedChild( unsigned id ) { // not important
		treeNode* tn = _treeNodes[ id ] ;
		assert( tn -> _use_num ) ;
		for( unsigned i = 0 ; i < tn -> _child.size() ; i++)
		{
			unsigned child_id = tn -> _child[i] ;
			if( _treeNodes[ child_id ] -> _use_num ) {
				tn -> remove_child( child_id ) ;
				i-- ;
			}
		}
	}
	void insert_NewDest( int ) ; 			// in every timeslot , some dest will join
	void insert_NewDest_BBSRT( int ) ;	// dest join in BBRST algorithm
	void delete_Dest( int ) ;				// in every timeslot , some dest will leave

	double dfs_branch() const ;						// return tree edgeCost
	double is_branch_node( unsigned id ) const ; // update every treeNode's bn_after , dest_after 

	// record treeNode's visited => other's path can not go through it
	void record_visited_to_treeHead( const unsigned& id , const unsigned& tree_head ) ;
	// use path to connect path , start_id is path[0]
	bool connect_path( const unsigned& start_id , const vector<unsigned>& path , bool reverse = 0 ) ;
	
	// tn_id want to change it's parent to new_parent
	void change_parent( const unsigned& id , const unsigned& new_parent );
	// use subTree._treeNodes to change tree._treeNodes => in rerouting
	void segtify_Node( const unsigned& sT_s , const vector< treeNode*>& sT_tns ) ;

	// check that if every node is connected and has only one upstream node => tree has loop or not
	bool checkTree( bool is_sub = 0 ) const ; 

	// use Destination to renew all node's use_num after rerout tree
	void renew_tree_usage() ;
	// from segmentTree leaves , change its downstream node to subtreeConnected
	void change_child_State_to_SubtreeConnected( unsigned id );
	
	// in checking that whether subtree is still being used or not
	bool check_subtree_path() const ;
	

	// print tree information
	void printTree_use_num() const ;
	void printTree_nodeUsed() const ;
	
	// if used = 1 => only print node that has been used
	// if you want to print tree that is not complete ( segmentTree ) , you need to let is_sub = 1 
	void printTree_topo( bool used = 0 , bool is_sub = 0 ) const ;
	void printTree_child( const vector<unsigned >& childs , unsigned level , bool used = 0 ) const ;
	
	void print_node_state() const ;
private:
	unsigned 				_segmentTreeInside ;
	unsigned 				_source 		;
	double 					_flow 		; 
	vector<treeNode*> 	_treeNodes 	;
};

class GraphMgr{
public:
	friend class algo ;
	GraphMgr(){
		_maxID = 0 ;
		_nodelist.clear() ;
		_edgelist.clear() ;
		srand( time( NULL ) ) ;
	}
	~GraphMgr(){
		while( !_nodelist.empty() ){
            Node* n = _nodelist.back() ;
            _nodelist.pop_back() ;
            delete n ;
      }
		while( !_edgelist.empty() ){
            Edge* e = _edgelist.back() ;
            _edgelist.pop_back() ;
            delete e ;
      }
	}
	void readGraph( string ) ; // READ IN file in /data
	Tree build_shortest_path_tree( unsigned root_id ) ;
	void shortest_path_proc( unsigned ) ;

	// use to find path from start to root => after shortest_path_proc( root )
	void getPath( vector<unsigned>& path , const unsigned& start ){ 
		assert( path.empty() ) ;
		path.push_back( start ) ;
		unsigned path_node_id = start ;
		while( true ) 
		{
			path_node_id = _nodelist[ path_node_id ] -> _parent ;
			if( path_node_id == UINT_MAX ) break ; // arrive tree's root 
			path.push_back( path_node_id ) ;
		}
	}

	void print_node() const {
		for( unsigned i = 0 ; i < _nodelist.size() ; i++ ){
			cout << i << ' ' << _nodelist[i] -> _cost << '\n' ;
		}
	}
private: 
	void init_node() {
		for( unsigned i = 0 ; i < _nodelist.size() ; i++ )
			_nodelist[i] -> _is_visit = 0 ; 
	}
	double Relax( const unsigned& , const unsigned& , const double& ) ;
	unsigned        _maxID ;
	vector< Node* > _nodelist ;
	vector< Edge* > _edgelist ;

};



