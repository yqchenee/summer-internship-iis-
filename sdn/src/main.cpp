#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <time.h>

#include "algo.h"
        
using namespace std;

int main()
{
    clock_t start , end ;
	string s ;
	cout << "file : " ;
    cin >> s ;
	unsigned mode ;
	cout << "mode : " ;
	cin >> mode ;
    start = clock() ;
	for( unsigned i = 0 ; i < RUN_TIME ; i++ ){
		algo Algo;
		Algo.readGraph( s ) ;
		switch( mode ){
			case 0 : Algo.SPT() ; break;
			case 1 : Algo.STRUS() ; break;
			case 2 : Algo.BBSRT() ; break;
		}
	}
    end = clock() ;
    cout << "used time : " << double( end - start )/ CLOCKS_PER_SEC << "s." << endl ;
}

void algo::BBSRT() {
	dataGen( SOURCE_NUM , JOIN , LEAVE , TIME_SLOTS , NODE_CAPACITY ) ;

    for( unsigned i = 0 ; i < _trees.size() ; i++ )
    {
        unsigned source = _trees[i] -> source ;
        Tree shortest_path_tree = _myGraph.build_shortest_path_tree( source );
		_trees[i] -> _tree = shortest_path_tree ;
    }
	TotalCost total_cost ;
	cout << "------------------" << endl;

	for( unsigned i = 0 ; i < TIME_SLOTS; i++)
	{
		cout << "TIME_SLOT : " << i << endl; 
		
		for( unsigned tree_num = 0; tree_num < _trees.size(); tree_num++)
		{
            const Tree* t_before = new Tree( _trees[ tree_num ]-> _tree ) ;
			
			leave(_trees[ tree_num ]-> _timeSlots[i].leave,  tree_num );
			if( i >= INITIAL_RUN ) {
				
				join_BBSRT (_trees[ tree_num ]-> _timeSlots[i].join,  tree_num );
			
				const Tree& t_after = _trees[ tree_num ] -> _tree ;
				// t_after.printTree_topo() ;
				total_cost._edgeCost += t_after.dfs_branch() ; // build multicast tree and return edgeCost 
				total_cost += calculate_tree_cost( *t_before , t_after ) ; // update transition rule
				
			}
			else join( _trees[ tree_num ]-> _timeSlots[i].join,  tree_num )	;		
		}
	}
	cout << "-------------------" << endl;
	total_cost._rule /= ( TIME_SLOTS - INITIAL_RUN +1 ) ; 
	cout << "BBSRT" << '\n' ;
	cout << total_cost << endl;
}

void algo::SPT() {
    dataGen( SOURCE_NUM , JOIN , LEAVE , TIME_SLOTS , NODE_CAPACITY ) ;
	
    for( unsigned i = 0 ; i < _trees.size() ; i++ )
    {
        unsigned source = _trees[i] -> source ;
        Tree shortest_path_tree = _myGraph.build_shortest_path_tree( source );
		_trees[i] -> _tree = shortest_path_tree ;
    }
    TotalCost total_cost ;
	cout << "------------------" << endl;

    for( unsigned i = 0 ; i < TIME_SLOTS; i++)
	{
		cout << "TIME_SLOT : " << i << endl; 

		for( unsigned tree_num = 0; tree_num < _trees.size(); tree_num++)
		{
            const Tree* t_before = new Tree( _trees[ tree_num ]-> _tree ) ;

			leave(_trees[ tree_num ]-> _timeSlots[i].leave,  tree_num );
			join (_trees[ tree_num ]-> _timeSlots[i].join,  tree_num );
			
			if( i >= INITIAL_RUN-1 )
			{
				const Tree& t_after = _trees[ tree_num ] -> _tree ;
				
				total_cost._edgeCost += t_after.dfs_branch() ; // build multicast tree and return edgeCost 
				total_cost += calculate_tree_cost( *t_before , t_after ) ; // update transition rule
			}
		}
    }
	total_cost._rule /= ( TIME_SLOTS - INITIAL_RUN +1 ) ; 
	cout << "-------------------" << endl;
	cout << "SPT" << '\n' ;
	cout << total_cost << endl;
}

void algo::STRUS() {
    dataGen( SOURCE_NUM , JOIN , LEAVE , TIME_SLOTS , NODE_CAPACITY ) ;
	// for each source , build the shortest path tree
    for( unsigned i = 0 ; i < _trees.size() ; i++ )
    {
        unsigned source = _trees[i] -> source ;
        Tree shortest_path_tree = _myGraph.build_shortest_path_tree( source );
		_trees[i] -> _tree = shortest_path_tree ;
    }
    TotalCost total_cost ;
	cout << "------------------" << endl;

    for( unsigned i = 0 ; i < TIME_SLOTS; i++)
	{
		cout << "TIME_SLOT : " << i << endl; 
		vector< const Tree* > trees_before ;
		trees_before.reserve( SOURCE_NUM ) ;

		for( unsigned tree_num = 0; tree_num < _trees.size(); tree_num++)
		{
			// RECORD tree that is in the last time_slot => calculate update , transition , rule
            const Tree* t_before = new Tree( _trees[ tree_num ]-> _tree ) ;
			trees_before.push_back( t_before ) ;

			leave(_trees[ tree_num ]-> _timeSlots[i].leave,  tree_num );
			join (_trees[ tree_num ]-> _timeSlots[i].join,  tree_num );
			
			if( check_if_subtree_remain( tree_num ) )   // true => tree_num not used its subtree
			{
				Key key( _trees[ tree_num ]-> _tree. _segmentTreeInside ) ;
				if( _mySeg.erase( key , tree_num ) ) { 	// this subtree is only used by the last one tree
					unsigned last_tree_id = _mySeg.get_last( key ) ;
					rebuild_tree( last_tree_id ) ;		// rebuild the last one
				}
				rebuild_tree( tree_num ) ;
			}
		}

		// i = initial run : 	dest start leaving 
		// i = initial run -1 : build the first multicast tree , and rerout by segmentTree
		if( i >= INITIAL_RUN-1 )
		{
			segmentTree() ;
			for( unsigned tree_num = 0; tree_num < _trees.size(); tree_num++)
			{
				const Tree& t_after = _trees[ tree_num ] -> _tree ;
				
				total_cost._edgeCost += t_after.dfs_branch() ; // build multicast tree and return edgeCost 
				total_cost += calculate_tree_cost( *( trees_before[ tree_num ] ) , t_after ) ; // update transition rule
			}

			// all segmentTree calculate cost
			total_cost += segTree_update_transition_Cost() ;
		}
		
    }
	total_cost._rule /= ( TIME_SLOTS - INITIAL_RUN +1 ) ; // average rule
	cout << "-------------------" << endl;
	cout << "STRUS" << '\n' ;
	cout << total_cost << endl;
}

void algo::segmentTree() 
{
	for( unsigned t = 1 ; t < _trees.size() ; t ++ )
	{
		// for each tree pair ( t , t-i ) find its common part
		for( unsigned i = 1 ; i <= t ; i++ ) 
		{
			subTree* sT = find_segmenttree( t ,  t -i ) ;
			if( sT == NULL ) continue ;

			// if segmentTree find , for each tree , check if it can use this segmentTREE
			unsigned segT_id = _mySeg.get_now_id() ;
			for( unsigned j = 0 ; j < _trees.size() ; j++ )  // for all tree ;
			{
				segmentTree_Pairing( segT_id ,  sT , t , t -i , j ) ;
			}
		}
	}

}

void algo::dataGen(int num_tree , int in_num_join , int in_num_leave ,\
                    int times , int node_cap )
{
    //  build dynamic tree with source and timeslot 
    for( int i = 0 ; i < num_tree ; i++)
	{
        unsigned src = ( unsigned( rand()) ) % _myGraph._maxID ;
        dynamicTree* newTree = new dynamicTree( src ) ;
        _trees.push_back( newTree ) ;
    }

	unsigned node_size = _myGraph._maxID +1 ;

    for( unsigned i = 0; i < _trees.size(); i++){
		vector < int > to_leave;

		for( unsigned t = 0; t < times; t++){
			time_slot this_time;
			const int& num_join = in_num_join ;
			const int& num_leave = in_num_leave ;

            // leave 
			if( t >= INITIAL_RUN ) {
				for(int k = 0; k < num_leave; k++) {
					if(!to_leave.empty()) {
						int index = rand() % to_leave.size();
						this_time.leave.push_back(to_leave[index]);
						to_leave.erase(to_leave.begin() + index);
					}
				}
			}

            // join 
			for(int k = 0; k < num_join; k++){
				int index ;
				do{
					index = rand() % node_size ;
					assert( index <= _myGraph._maxID ) ;
					if( index != _trees[i]->  source ) 
					{
						this_time.join.push_back( index );
						to_leave.push_back( index );
					}
				}while( index == _trees[i]-> source ) ;
			}
			_trees[i]-> _timeSlots.push_back(this_time);
		}
	}
    // print_time_slots();
}