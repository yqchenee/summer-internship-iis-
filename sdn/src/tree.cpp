#include "graph.h"

// in every timeslot , some dest will join
// when dest_id is join , update node's use_num along the path to source  
void Tree::insert_NewDest( int dest_id ) 
{
    assert( dest_id >= 0 ) ;
    //cout <<"join dest : " << dest_id << endl ;
    
    treeNode* treeN = _treeNodes[ dest_id ] ; // dest tree node 
    treeNode* n_treeN ;  // next tree node

    treeNode_State& tn_state = treeN -> _state ;
    assert( tn_state != Source ) ;
    tn_state = ( tn_state <= 2 ) ? Destination : 
                    ( tn_state <= 4 ) ? SubtreeConnectedDest :
                    ( tn_state == 5 ) ? SubTreeSource : tn_state ;

    treeN -> _use_num += 1 ;
    treeN -> _num_dest += 1 ;

    unsigned parent  = treeN -> _parent ;
    n_treeN = ( parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ; 
    // update tree path utility
    while( n_treeN )
    {
        n_treeN -> _use_num += 1 ;
        parent  = n_treeN -> _parent ;
        n_treeN = (parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ;     
    }
}

// dest join in BBRST algorithm
void Tree::insert_NewDest_BBSRT( int dest_id )
{
    assert( dest_id >= 0 ) ;
    //cout <<"join " << dest_id << endl ;
    
    treeNode* treeN = _treeNodes[ dest_id ] ; // dest tree node 
    treeNode* n_treeN ;  // next tree node

    treeN -> _state = Destination ;
    treeN -> _use_num += 1 ;
    treeN -> _num_dest += 1 ;

    unsigned parent  = treeN -> _parent ;
    n_treeN = ( parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ; 
    // update tree path utility
    while( n_treeN )
    {
        n_treeN -> _use_num += 1 ;
        parent  = n_treeN -> _parent ;
        n_treeN = (parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ;     
    }
}

// in every timeslot , some dest will leave
// when dest_id isleave , update node's use_num along the path to source 
void Tree::delete_Dest( int dest_id )
{
    assert( dest_id >= 0 ) ;
    //cout << "leave dest : " << dest_id << endl;
    
    treeNode* treeN = _treeNodes[ dest_id ] ; // dest tree node 
    treeNode* n_treeN ;  // next tree node
    assert( treeN -> _num_dest ) ;
    
    treeN -> _use_num -= 1 ;
    treeN -> _num_dest -= 1 ;

    if( treeN -> _num_dest == 0 ) {
        treeNode_State& tn_state = treeN -> _state ;
        assert( tn_state != Source ) ;
        tn_state = ( tn_state <= 2 ) ? Normal : 
                    ( tn_state <= 4 ) ? SubtreeConnectedNode :
                    ( tn_state == 5 ) ? SubTreeSource : tn_state ;
    }

    unsigned parent  = treeN -> _parent ;
    n_treeN = (parent != UINT_MAX ) ? _treeNodes[ parent ] : NULL ;
    // update tree path utility
    while( n_treeN )
    {
        n_treeN -> _use_num -= 1 ;
        parent  = n_treeN -> _parent ;
        n_treeN = ( parent != UINT_MAX ) ? _treeNodes[ parent ] : NULL ;
    }
}

// use dfs from source
// update every treeNode's bn_after , dest_after
double Tree::dfs_branch() const
{
    clear_tree_node() ;  // init every node bn_after , dest_after
    return _flow * is_branch_node( _source ) ; // edgecost
}

// update every treeNode's bn_after , dest_after by dfs
double Tree::is_branch_node( unsigned id ) const 
{
    double treeCost = 0 ;
    treeNode* tn = _treeNodes[id] ;
    assert( tn != NULL ) ;
    
    // id is destination
    for( unsigned i = 0 ; i < tn -> _num_dest ; i++ )
        tn -> dest_after.push_back( id ) ;

    vector<unsigned > child_vec = get_usedChild( id ) ;
    while( !child_vec.empty() )
    {
        unsigned child_id = child_vec.back() ;
        treeNode* child_node = _treeNodes[ child_id ] ;
        child_vec.pop_back() ;
        // child_id dest num + outgoing edge >= 2  ==>> child_id is branch state node
        vector<unsigned> used_child_vec = get_usedChild( child_id ) ;; 
        if( (used_child_vec.size() + child_node-> _num_dest ) >= 2 )
        {
            tn -> bn_after.push_back( child_id ) ;
            treeCost += ( child_node -> _cost - tn -> _cost ) ;
            treeCost += is_branch_node( child_id ) ;
        }
        // child_id is destination and no out going edge
        else if( child_node -> _num_dest ){
            assert( child_node -> _num_dest == 1 ) ;
            tn -> dest_after.push_back( child_id ) ;
            treeCost += ( child_node-> _cost - tn -> _cost ) ;
        }
        // child_id is not destination and only one outgoing edge 
        else {
            assert( used_child_vec.size() == 1 ) ;
            child_vec.push_back( used_child_vec[0] ) ;
        }
    }
    return treeCost ;
}

// record treeNode's visited => path can not go through it
void Tree::record_visited_to_treeHead( const unsigned& id , const unsigned& tree_head ) 
{
   _treeNodes[id] -> _is_visit = 1 ;
   if( id == tree_head ) return ;
   assert( id != _source ) ;
   unsigned parent = _treeNodes[ id ] -> _parent ;
   treeNode* tn_parent = _treeNodes[ parent ] ;
   record_visited_to_treeHead( parent , tree_head ) ;
}
// use path to connect path , start_id is path[0]
bool Tree::connect_path( const unsigned& start_id , const vector<unsigned>& path , bool reverse  ) {
   assert( start_id == path[0] ) ; 
   unsigned i = 0 ;
   for( ; ; ) {
      unsigned j = i + 1 ; // path.size() >= 2 
      if( _treeNodes[ path[j] ] -> _is_visit == 1 ) {
         return false ; 	// j = 0 => tree itself
      }
      _treeNodes[ path[j] ] -> _is_visit = 1 ; // other path can not go through node which has been gone through
      if( reverse ) change_parent( path[j] , path[i] ) ;
      else change_parent( path[i] ,path[j] ) ; // j is i new parent
      i = j ;
      if( j == path.size() - 1 ) break ; 
   }
   return true ;
}

// tn_id want to change it's parent to new_parent
void Tree::change_parent( const unsigned& id , const unsigned& new_parent )
{
   if( id == new_parent ) return ; 
   static bool uncertain = 0 ;  // not important
   treeNode* const tn_id = _treeNodes[ id ] ;    
   treeNode* const ori_p = _treeNodes[ tn_id -> _parent ] ;
   treeNode* const new_p = _treeNodes[ new_parent ] ;
   if( new_parent == tn_id -> _parent ) return ; // new parent is the same as the origin one's

   ori_p -> remove_child( id , uncertain ) ;
   if( new_p -> _parent == id ) {  // if new path and origin path are in reverse direction
      tn_id -> remove_child( new_parent ) ;
      uncertain = 1 ;
   }
   else uncertain = 0 ;
   tn_id -> _parent = new_parent ;
   new_p -> add_child( id ) ;
}

// use subTree._treeNodes to change tree._treeNodes => in rerouting
void Tree::segtify_Node( const unsigned& sT_s , const vector< treeNode*>& sT_tns ) {
   queue< unsigned > q_container ;
   q_container.push( sT_s ) ;
   assert( sT_tns[ sT_s ]-> _state == SubTreeSource ) ;

   while( !q_container.empty() ) {
      unsigned subTree_node_id = q_container.front() ;
      q_container.pop() ;
      treeNode* sT_tn = sT_tns[ subTree_node_id ] ;
      assert( sT_tn != NULL ) ;
      
      const vector<unsigned>& v_childs = sT_tn-> _child ; // used
      queue<unsigned> q_childs = vector_to_queue( v_childs ) ;
      append_queue( q_container , q_childs ) ;
         
      if( v_childs.size() >= 2 ) _treeNodes[ subTree_node_id ] -> _state = SubTreeBranch ;
      else if( v_childs.size() == 1 )_treeNodes[ subTree_node_id ] -> _state = SubTreeNormal ;
      else _treeNodes[ subTree_node_id ] -> _state = SubTreeLeaf ;

      unsigned parent = sT_tn -> _parent ;
      if( parent != UINT_MAX ) {
         change_parent( subTree_node_id , parent ) ; // use subTree._treeNodes to change _treeNodes parent
      }
      else assert( sT_tn -> _state == SubTreeSource ) ;
   }
   _treeNodes[ sT_s ] -> _state = SubTreeSource ;
}

// use Destination to renew all node's use_num after rerout tree
void Tree::renew_tree_usage() {
   init_treeNode_usenum() ;
   for( unsigned i = 0 ; i < _treeNodes.size() ; i++ )
   {
      unsigned un = _treeNodes[i] -> _num_dest ;
      if( un ) 
      {
         _treeNodes[i] -> _use_num += un ;
         assert( _treeNodes[i] -> _state == Destination || 
                  _treeNodes[i] -> _state >= 4 ) ;

         unsigned  parent  = _treeNodes[i] -> _parent ;
         treeNode* n_treeN = ( parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ; 
         
         while( n_treeN ) // update it's parent's use_num to source
         {	
            n_treeN -> _use_num += un ;
            parent  = n_treeN -> _parent ;
            n_treeN = (parent!= UINT_MAX ) ? _treeNodes[ parent ] : NULL ;     
         }
      }
      // change node which is connected to subtreeLeaf to subtreeConnected node
      if( _treeNodes[i] -> _state == SubTreeLeaf ) 
         change_child_State_to_SubtreeConnected( i ) ;
   }
}
// from segmentTree leaves , change its downstream node to subtreeConnected
void Tree::change_child_State_to_SubtreeConnected( unsigned id ){
   vector<unsigned > childs = _treeNodes[id] -> _child ;
   
   while( !childs.empty() ){
      unsigned node_id = childs.back() ;
      childs.pop_back() ;
      treeNode* tn = _treeNodes[ node_id ] ;
      if( tn -> _num_dest ) tn-> _state = SubtreeConnectedDest ;
      else tn -> _state = SubtreeConnectedNode ;
      
      for( unsigned i = 0 ; i < tn-> _child.size() ; i++ ) {
         childs.push_back( tn-> _child[i] ) ;
      }
   }
}

// in checking that whether subtree is still being used or not
bool Tree::check_subtree_path() const {  
   if( _segmentTreeInside == 0 ) return false ;
   bool other_node_not_use = 0 ;
   for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
      treeNode* tn = _treeNodes[i] ;
      if( tn -> _state >= 5 ) { // subtree
         if( tn -> _use_num == 0 ) {
            if( tn -> _state != SubTreeLeaf ) {
               other_node_not_use = 1 ;
            }
            else return true ; // subtree not use
         }
      }
   }
   // if other node not use( subtree not used ) , leaves must be not used either
   assert( !other_node_not_use ) ; 
   return false ;
}

// check that if every node is connected and has only one upstream node => tree has loop or not
bool Tree::checkTree( bool is_sub ) const {
    if( is_sub ) return true ;
    bool* ref = new bool [ _treeNodes.size() ] ;
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
        ref[i] = 0 ;
    }

    ref[ _source ] = 1 ;
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
        const vector<unsigned>& tn_c = _treeNodes[i]-> _child ;
        for( unsigned j = 0 ; j < tn_c.size() ; j++ ) {
            unsigned child = tn_c[j] ;
            assert( ref[child] == 0 ) ;
            ref[child] = 1 ;  
        } 
    }
    
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ ) {
        if( ref[i] == 0 ) return false ;
    }
    return true ;
}

// print tree information
void Tree::printTree_topo( bool used , bool is_sub ) const {
    if( checkTree( is_sub ) == 0 ) assert(0) ;
    unsigned level = 1 ;
    cout << _source << endl ;
    const vector<unsigned >& childs = _treeNodes[_source] -> _child ;
    printTree_child( childs , level , used ) ;
}
void Tree::printTree_child( const vector<unsigned >& childs , unsigned level , bool used ) const {
    for( unsigned i = 0 ; i < childs.size() ; i++ ){
        if( _treeNodes[ childs[i] ] -> _use_num == 0 && used ) continue ;
        for( unsigned l = 0 ; l < level ; l++ )
            cout << "  " ;
        
        cout << childs[i] <<" ( " << _treeNodes[childs[i] ] -> _use_num << " ) "<<  endl;
        
        const vector<unsigned >& v = _treeNodes[ childs[i] ] -> _child ;
        printTree_child( v , level+1 ) ;
    } 
}

void Tree::printTree_nodeUsed() const 
{
    cout << "root: " << _source << "=====" << endl ; 
    // printTree_topo() ;
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ )
    {
        treeNode* tn = _treeNodes[i] ;
        cout << i << ' ' << tn -> _use_num ;
        switch( _treeNodes[i]-> _state ) {
            case 0 : cout << " source" ; break ;
            case 1 : cout << " Normal" ; break ;  
            case 2 : cout << " Destination" ; break ;
            case 3 : cout << " SubtreeConnectedNode" ; break ; 
            case 4 : cout << " SubtreeConnectedDest" ; break ; 
            case 5 : cout << " SubTreeSource" ; break ; 
            case 6 : cout << " SubTreeNormal" ; break ; 
            case 7 : cout << " SubTreeBranch" ; break ; 
            case 8 : cout << " SubTreeLeaf" ; break ; 
        }
        cout << '\n' << "  bn_after: " ;
        for( unsigned j = 0 ; j < tn -> bn_after.size() ; j++ )
        {
            cout << tn -> bn_after[j] << ' ' ; 
        }
        cout << '\n' << "  dest_after: ";
        for( unsigned j = 0 ; j < tn -> dest_after.size() ; j++ )
        {
            cout << tn-> dest_after[j] << ' ' ;
        }
        cout << '\n' ;
    }
}

void Tree::printTree_use_num() const
{
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ )
    {
        cout << i << " used: " << _treeNodes[i]-> _use_num << '\n' ; 
    }
}
void Tree::print_node_state() const 
{
    for( unsigned i = 0 ; i < _treeNodes.size() ; i++ )
    {
        cout << i << " : " ;
        switch( _treeNodes[i]-> _state ) {
            case 0 : cout << "source" ; break ;
            case 1 : cout << "Normal" ; break ;  
            case 2 : cout << "Destination" ; break ;
            case 3 : cout << "SubtreeConnectedNode" ; break ; 
            case 4 : cout << "SubtreeConnectedDest" ; break ; 
            case 5 : cout << "SubTreeSource" ; break ; 
            case 6 : cout << "SubTreeNormal" ; break ; 
            case 7 : cout << "SubTreeBranch" ; break ; 
            case 8 : cout << "SubTreeLeaf" ; break ; 
        }
        cout << '\n' ;
    }
}