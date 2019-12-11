#include <vector>
#include <cassert>
#include <utility> 
using namespace std ;

class Heap{
public:
    Heap(){}
    void Build_Min_Heap() ;
    bool empty(){ return _data.empty() ; }
    unsigned size() { return _data.size() ; }
    void insert( const pair<double , unsigned >& ) ;
    void insert( const double& , const unsigned& ) ;
    pair< double , unsigned > pop_min() ;
    void decrease( const pair< double , unsigned >& ) ;
    void decrease_key( unsigned index , const double& val ) ;
    unsigned find( const pair<double , unsigned >& key ) ;
    void print() const {
        for( unsigned i = 0 ; i < _data.size() ; i++ )
        {
            cout << _data[i].first << ' ' << _data[i].second << endl;
        }
    } 
private:
    vector< pair< double , unsigned> > _data ;
    void Min_Heapify( unsigned index = 0 ) ;
};

pair< double , unsigned > Heap::pop_min()
{
    int heapSize = _data.size() ;
    assert( heapSize ) ;
    pair< double , unsigned > ret = _data[0] ;
    swap( _data[0] , _data[ heapSize -1 ] ) ;
    _data.pop_back() ;

    Min_Heapify() ;

    return ret ;
}

void Heap::decrease( const pair<double , unsigned >& key )
{
    unsigned index = find( key ) ;
    decrease_key( index , key.first ) ;
}
void Heap::decrease_key( unsigned index , const double& val )
{
    // unsigned index = find( key ) ;
    assert( val < _data[index].first ) ;
    _data[ index ].first = val ;

    #define Parent(i) ( ( i%2 == 0 ) ? i/2 -1 : i/2 ) 
    while( index > 0 && _data[Parent( index )].first > _data[index].first )
    {
        swap( _data[Parent( index )] , _data[index]) ;
        index = Parent( index ) ;
    }
}

void Heap::insert( const pair<double , unsigned >& n_data ) 
{
    pair< double , unsigned > t ( DBL_MAX , n_data.second ) ;
    _data.push_back( t ) ;
    decrease_key( _data.size() -1 , n_data.first ) ;
}
void Heap::insert( const double& val , const unsigned& index )
{
    pair< double , unsigned > t ( val , index ) ;
    _data.push_back( t ) ;
}


void Heap::Min_Heapify( unsigned index )
{
    unsigned l = (2 * index) + 1 ; // left leaf
    unsigned r = (2 * index) + 2 ; // right leaf
    unsigned heapSize = _data.size() ;
    unsigned least = index ; 
    if( l < heapSize )
    {
        if( _data[l].first < _data[index].first ) least = l;
    }
    if( r < heapSize )
    {
        if( _data[r].first < _data[least].first ) least = r;
    }
    if( least != index )
    {
        swap( _data[index] , _data[least]);
        Min_Heapify( least );
    }
}

void Heap::Build_Min_Heap()
{
    unsigned heapSize = _data.size(); 
    assert( heapSize ) ;
    if( heapSize == 1 ) return ; 
    
    heapSize = ( heapSize % 2 ) ? heapSize-3 : heapSize -2 ;
    for( unsigned i = (heapSize/2) ; ; i-- )
    {
        Min_Heapify( i );
        if( i == 0 ) break ;
    }
}


unsigned Heap::find( const pair<double , unsigned >& key )
{   
    for( unsigned i = 0 ; i < _data.size() ; i++ )
    {
        if( _data[i].second == key.second ) return i ;
    }
    assert(0) ;
}