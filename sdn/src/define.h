#define RUN_TIME  1		
#define FLOW_SIZE 10		// maximum flow size => if(10) => 1 ~ 10	

#define INITIAL_RUN 15 	// total destination 
#define TIME_SLOTS 100 + INITIAL_RUN
#define SOURCE_NUM 50
#define JOIN 1
#define LEAVE 1  

#define MIN_SEGMENTREE_SIZE 5

#define THRESHOLD_RATIO 35
#define ALPHA 0.65
#define BETA  1
#define RHO   0.25
#define TAU   0.55

#define NODE_CAPACITY 10


enum treeNode_State{
	Source , // 0
	Normal , // 1
	Destination , // 2
   SubtreeConnectedNode , // 3
	SubtreeConnectedDest , // 4
	SubTreeSource , // 5
	SubTreeNormal , // 6
	SubTreeBranch , // 7
	SubTreeLeaf     // 8
};

#include <vector> 
#include <queue>
#include <algorithm>
using namespace std ;
template < class T > 
bool same_vector( const vector<T>& a_vec , const vector<T>& b_vec )
{
	if( a_vec.size() != b_vec.size() ) return false ;
	for( unsigned i = 0 ; i < a_vec.size() ; i++ )
	{
		if( find( b_vec.begin() , b_vec.end() , a_vec[i] ) == b_vec.end() ) return false ; 
	} 
	return true ;
}
template < class T >
vector<T> find_common( const vector<T>& va , const vector<T>& vb )
{
	vector< T > ret ;
	for( unsigned i = 0 ; i < va.size() ; i++ )
	{
		for( unsigned j = 0 ; j < vb.size() ; j++ )
		{
			if( va[i] == vb[j] )  
			{
				ret.push_back( va[i] ) ;
		//		cout << "push " << va[i] << endl;
				break ;
			}
		}
	}
	return ret ;
}
template < class T > 
void append_vector( vector<T>& master , vector<T>& sl ) 
{
	master.insert( master.end() , sl.begin() , sl.end() );	
}
template < class T > 
void append_queue( queue<T>& master , queue<T>& sl )
{
	while( !sl.empty() ) {
		master.push( sl.front() ) ;
		sl.pop();
	}
}
template < class T >
vector<T> queue_to_vector( queue< T >& q_container )
{
	vector< T > ret ;
	while( !q_container.empty() ){
		ret.push_back( q_container.front() ) ;
		q_container.pop() ;
	}
	return ret ;
}

template < class T >
queue<T> vector_to_queue( const vector<T>& vec )
{
	queue<T> ret ;
	for( unsigned i = 0 ; i < vec.size() ; i++ )
	{
		ret.push( vec[i] ) ;
	}
	return ret ;
}

template < class T >
void reverse_vector( vector<T>& vec ) 
{
	unsigned mid = vec.size() / 2 ;
	unsigned max = vec.size()-1 ;
	for( unsigned i = 0 ; i < mid ; i++ )
	{
		T temp = vec[i] ;
		vec[i] = vec[ max-i ] ;
		vec[ max-i ] = vec[i] ;
	}
}

