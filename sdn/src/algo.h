#include "graph.h"
#include "util/hash.h"
#include <iostream>

#include <string.h>
#include <stdlib.h>


class time_slot{
public:
    vector<int> join ;
    vector<int> leave ;
};

class dynamicTree{
public:
    dynamicTree( unsigned s ) : source(s) {}
    unsigned            source ;
    Tree                _tree ;
    vector< time_slot > _timeSlots ; // record every time_slot => which has to leave or join
};

class SegMgr{
public:
    SegMgr() : now_id( 1 ) {}
    void init( unsigned num_buckets ) { all_segTree.init( num_buckets ); } 
    unsigned get_now_id(){
        return( now_id++ ) ;
    }
    void push( const Key& k , const unsigned& tree_id ) {
        all_segTree.push( k , tree_id ) ;
    }
    bool erase( const Key& k , const unsigned& tree_id ) {
        return all_segTree.erase( k , tree_id ) ;
    }
    unsigned get_last( const Key& k ) {
        return all_segTree.get_last( k ) ;
    }
    vector< unsigned >& get_used_after( const Key& k ) {
        return all_segTree.get_used_after( k ) ;
    }
    vector< unsigned >& get_used_before( const Key& k ) {
        return all_segTree.get_used_before( k ) ;
    }
    vector< unsigned >& get_same_node( const Key& k ) {
        return all_segTree.get_same_node( k ) ;
    }
    vector< unsigned >& get_tree_used( const Key& k ) {
        return all_segTree.get_tree_used( k ) ;
    }
private:
    unsigned now_id ;
    HashMap< Key , sT_info > all_segTree ;
};

class subTree{
public:
    subTree( Tree* new_tree ){
        _subTree_size = 0 ;
        _subtree_cost = 0 ;
        _leaves.clear() ;
        _tree = *new_tree ;

        for( unsigned i = 0 ; i < _tree. _treeNodes.size() ; i++ )
        {
            treeNode* sT_tn = _tree. _treeNodes[i] ;
            if( sT_tn != NULL )
            {   
                _subTree_size ++ ;
                const vector<unsigned>& child_vec = sT_tn -> _child ; 
                for( unsigned j = 0 ; j < child_vec.size() ; j++) {
                    _subtree_cost += (_tree.get_treeNode_cost( child_vec[j] ) - _tree.get_treeNode_cost( i ) ) ;
                }

                assert( sT_tn -> _state >= 5 ) ;
                if( sT_tn -> _state == SubTreeSource ) continue ;
                switch( child_vec.size() ) {
                    case 0 :    sT_tn -> _state = SubTreeLeaf ; 
                                _leaves.push_back( i ) ;
                                break ;
                    case 1 :    sT_tn -> _state = SubTreeNormal ; break ;
                    default :   sT_tn -> _state = SubTreeBranch ;
                }
            }
        } 
    }

    unsigned         _subTree_size ;
    double           _subtree_cost ; // _flow = 1 
    vector<unsigned> _leaves ;
    Tree             _tree ;
};

class algo{
public:
    algo(){ 
        unsigned n = SOURCE_NUM / 25 ; // hash's num_bucket
        n = ( n > 0 ) ? n : 1 ;
        _mySeg.init( n ); 
    } 
    ~algo(){
        while( !_trees.empty() ){
            dynamicTree* dt = _trees.back() ;
            _trees.pop_back() ;
            delete dt ;
        }
    }
    void readGraph( const string& s) ;
    void SPT() ;
    void STRUS() ;
    void BBSRT() ;
private:
    // calculate cost( update , transition , rule ) for tree before and after the tree routing
    TotalCost calculate_tree_cost( const Tree& t_before , const Tree& t_after ) ;
    TotalCost calculate_segtreeInside_cost( const Tree& t_before , const Tree& t_after ) ;
    // calculate cost( update , transition , rule ) for each node before and after the tree routing
    void calculate_each_tn_Cost( TotalCost& ret , const treeNode* tn_before , const treeNode* tn_after , bool segConnected = 0 ) ;
    bool is_child_same( const treeNode* tn_before , const treeNode* tn_after ) ; // check if bn_after and dest_after is same or not
    
    // all segmentTree calculate cost
    TotalCost segTree_update_transition_Cost() ;

    // for each tree , assign its timeslots( which to join or leave in a time )
    void dataGen( int num_tree , int num_join , int num_leave , int times , int node_cap) ;
    void join (const vector < int >& dsts, int tree_index );
	void leave(const vector < int >& dsts, int tree_index );
    void join_BBSRT (const vector < int >& dsts, int tree_index );

	// in checking that whether subtree is still being used or not
    bool check_if_subtree_remain( const unsigned& tree_num ) ; 

    // after rerout , if subtree is not used , rebuild it by shortest_path_tree
    void rebuild_tree( const unsigned& tree_index ) ; 
    

    void segmentTree() ;
    // for tree a and tree b , find it common part( segment tree )
    subTree* find_segmenttree( const unsigned& a , const unsigned& b );
    // for segnentTree sT , check whether tree t can use this sT or not
    void segmentTree_Pairing( const unsigned& segT_ID , const subTree* const sT , 
                        const unsigned& st_o1, const unsigned& st_o2 , const unsigned& t ) ;
    void init_pairing( const subTree* const sT ) ;  // not important

    // check if there is enough child to connect to subtree or not 
    bool check_rerouting( double& threshold_Cost , queue<unsigned>& tree_child_node , 
                            const subTree* const sT , const unsigned& tree_id ) ;
    // compare with threshold to decide if it could replace by subtree
    bool check_threshold( vector< vector<unsigned >>& all_path , 
                            const double& origin_Cost , const vector<unsigned>& need_connect_node ,	
                            const double& to_head_Cost , const subTree* const sT  , const unsigned& tree_id ) ;
    // if all path , to_head_path contains sT's node return false
    bool check_path     ( const vector< vector<unsigned >>& all_path , const vector<unsigned>& to_head_path , 
                            const subTree* const sT ) ;
    // use all_path , to_head_path , sT , to rerout tree
    bool rerouting      ( const vector< vector<unsigned >>& all_path , const vector<unsigned>& to_head_path , 
                            const subTree* const sT , const unsigned& tree_id ) ;
    // origin tree need to segtify, too 
    bool segtifyTree    ( const unsigned& segT_ID , const subTree* const sT , 
							const unsigned& st_o1 , const unsigned& st_o2 ) ;
    // from subtree root , find min-cost node in tree and return min_cost id and path
    pair< vector<unsigned> , double > find_min_path_and_Cost ( const unsigned& root , const unsigned& tree_id ) ;
    
    // from leaves to need connected node , construct matrix to record each leaves to each node path and cost 
    void constructCostMatrix( pair< vector<unsigned> , double >**& new_path_cost , const unsigned& tree_id ,\
                            const vector<unsigned>& subtree_leaves , const vector<unsigned>& need_connect_node ) ;
    
    void print_time_slots();

    SegMgr                  _mySeg   ;
    GraphMgr                _myGraph ;
    vector< dynamicTree* >  _trees   ;
};

