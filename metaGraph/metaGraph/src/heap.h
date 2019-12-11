#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <cassert>
#include <utility> 
#include <cfloat>

using namespace std ;

class Heap{
public:
    Heap(){}
    void Build_Min_Heap() ;
    bool empty(){ return _data.empty() ; }
    unsigned size() { return _data.size() ; }
    void insert( const pair<float , unsigned >& ) ;
    void insert( const float& , const unsigned& ) ;
    pair< float , unsigned > pop_min() ;
    float get_min() { return _data[0].first ; } 
    void decrease( const pair< float , unsigned >& ) ;
    void decrease_key( unsigned index , const float& val ) ;
    unsigned find( const pair<float , unsigned >& key ) ;
    
    vector< pair< float , unsigned> > getallData() const {
      return _data ;
    }
private:
    vector< pair< float , unsigned> > _data ;
    void Min_Heapify( unsigned index = 0 ) ;
};

pair< float , unsigned > Heap::pop_min()
{
    int heapSize = _data.size() ;
    assert( heapSize ) ;
    pair< float , unsigned > ret = _data[0] ;
    swap( _data[0] , _data[ heapSize -1 ] ) ;
    _data.pop_back() ;

    Min_Heapify() ;

    return ret ;
}

void Heap::decrease( const pair<float , unsigned >& key )
{
    unsigned index = find( key ) ;
    decrease_key( index , key.first ) ;
}
void Heap::decrease_key( unsigned index , const float& val )
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

void Heap::insert( const pair<float , unsigned >& n_data ) 
{
    pair< float , unsigned > t ( FLT_MAX  , n_data.second ) ;
    _data.push_back( t ) ;
    decrease_key( _data.size() -1 , n_data.first ) ;
}
void Heap::insert( const float& val , const unsigned& index )
{
    pair< float , unsigned > t ( val , index ) ;
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


unsigned Heap::find( const pair<float , unsigned >& key )
{   
    for( unsigned i = 0 ; i < _data.size() ; i++ )
    {
        if( _data[i].second == key.second ) return i ;
    }
    assert(0) ;
}

#endif // !HEAP_H