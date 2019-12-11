#include <iostream>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <cfloat>
#include "graph.h"
#include "util/heap.h"

using namespace std;

// READ IN file in /data
// data format : 
// edge_id1    node1    node2   edge1_weight
// edge_id2    node1    node2   edge2_weight
// 
void GraphMgr::readGraph( string infile ) 
{
    ifstream Input_file;
    infile = "data/" + infile ;
    Input_file.open(infile);
    if( !Input_file.is_open() ){
        cout << infile << " is not exist." << endl;
        return ;
    }
    else 
    {
        unsigned dd , id1 , id2 ;
        double   cost ;
        ////  assign maxid
        while( Input_file >> dd )
        {
            Input_file >> id1 >> id2 >> cost ;
            if( _maxID < id1 ) _maxID = id1 ;
            if( _maxID < id2 ) _maxID = id2 ;
        }
        // use max_id to assign node
        for( int i = 0 ; i <= _maxID ; i++ )
        {
            Node* newNode = new Node( i ) ;  // node id => _nodelist[id]
            _nodelist.push_back( newNode );
        } 
        Input_file.close();
    }

    Input_file.open(infile);
    if( Input_file.is_open() ){
        unsigned dd , id1 , id2 ;
        double   cost ;
        // assign edge
        while( Input_file >> dd )
        {
            Input_file >> id1 >> id2 >> cost ;
            Edge* e1 = new Edge( id1 , id2 , cost );
            _nodelist[ id1 ]-> new_Edge( e1 ) ;
            _nodelist[ id2 ]-> new_Edge( e1 ) ;
            _edgelist.push_back( e1 ) ;
        }
        Input_file.close();
    }
}

// from root , build shortest path tree 
Tree GraphMgr::build_shortest_path_tree( unsigned root )
{
    shortest_path_proc( root ) ; // update all node's cost from root , and parent

    Tree tree ;
    tree._source = root ;
    for( unsigned index = 0 ; index < _nodelist.size() ; index++ )
    {
        Node* n = _nodelist[index];
        treeNode* tn = new treeNode( n );
        // for every edge , if I( index ) is otherside's parent , other is my child 
        for( unsigned j = 0 ; j < n -> _outEdges.size() ; j++ )
        {
            Edge* e = n -> _outEdges[j] ;
            unsigned other = e -> otherside( index ) ;
            if( _nodelist[ other ] -> _parent == index ) {
                tn -> new_child( other ) ;
            }
        }
        tree._treeNodes.push_back( tn );    
    }
    tree._treeNodes[ root ] -> get_treeNode_state() = Source ;

    cout <<"build shortest tree "<< '\n';
    cout <<"root = " << root << '\n';
    cout <<"flow = " << tree._flow << '\n';

    return tree ;
}

// dijksta algorithm
void GraphMgr::shortest_path_proc( unsigned root)
{
    init_node() ;
    Heap priority_queue ; // to store every node , and pop node which is min cost 
    // init
    for( unsigned i = 0 ; i < _nodelist.size() ; i++){
        _nodelist[i]-> _cost = DBL_MAX ;
        _nodelist[i]-> _parent = UINT_MAX ; 
        priority_queue.insert( DBL_MAX , i ) ; // let every node cost is DBL_MAX , and put in priority_queue
    }
    _nodelist[ root ] -> _cost = 0 ;           // root cost is 0
    priority_queue.insert( 0 , root ) ;
    priority_queue.Build_Min_Heap() ; 
    // init end

    while( !priority_queue.empty() )
    {
        // pair( cost , id )
        pair< double , unsigned > now = priority_queue.pop_min() ; // now is the minimum one
        unsigned index = now.second ;
        if( _nodelist[ index ]-> _is_visit ) continue ;

        _nodelist[ index ] -> _is_visit = true ; 
        // find all its outEdge and update it cost 
        for( unsigned i = 0 ; i < _nodelist[ index ] -> _outEdges.size() ; i++ )
        {
            Edge* e = _nodelist[index] -> _outEdges[i] ;
            unsigned otherside = e -> otherside( index ) ;
         
            double cost =  Relax( index , otherside , e -> getCost() ) ;
            if( cost != -1 )  // cost is updated in Relax
            {
                pair< double , unsigned > t( cost , otherside ) ;
                priority_queue.insert( t ) ;
            }
        }
    }
}

// if node become less cost , return its new cost 
double GraphMgr::Relax( const unsigned& s , const unsigned& d , const double& c )
{
    Node* n_s = _nodelist[s] ;
    Node* n_d = _nodelist[d] ;
    if( n_d -> _cost > ( n_s -> _cost + c ) )
    {
        n_d -> _cost = ( n_s -> _cost + c ) ;
        n_d -> _parent = s ;
        return n_d -> _cost ;
    }
    return -1 ;
} 
