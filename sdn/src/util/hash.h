#include <vector>

using namespace std;

class Key{
public:
   Key( const size_t& id ): _id(id) {}
   size_t operator() () const{ return ( _id ); }
   bool operator == ( const Key& k ) const { 
      if( k._id == _id ) 
         return true;
      
      else return false;
   }
private:
   size_t _id ;
};
class sT_info{
public:
   sT_info( const unsigned& t_id ) { 
      _used_after.clear() ;
      _used_before.clear() ;
      _same_tn.clear() ;
      _which_tree_used.clear() ;
      _which_tree_used.push_back( t_id ) ;
   }
   void push( const unsigned& t_id ) { _which_tree_used.push_back( t_id ); }
   bool erase( const unsigned& t_id ) { 
      unsigned i = 0 ;
      for( ; i < _which_tree_used.size() ; i++ ) {
         if( _which_tree_used[i] == t_id ) 
         {
            _which_tree_used.erase( _which_tree_used.begin() + i ) ;
            if( _which_tree_used.size() == 1 ) return true ;
            else return false ;
         }
      }
      assert( 0 ) ; // not find
   }
   unsigned get_last() {
      assert( _which_tree_used.size() == 1 ) ;
      return _which_tree_used.front() ;
   }
   vector< unsigned >& get_used_after() {
      return _used_after ;
   }
   vector< unsigned >& get_used_before() {
      return _used_before ;
   }
   vector< unsigned >& get_same_node() {
      return _same_tn ;
   }
   vector< unsigned >& get_tree_used() {
      return _which_tree_used ;
   }
private:
   vector< unsigned >   _used_after ;
   vector< unsigned >   _used_before ;
   vector< unsigned >   _same_tn ;
   vector< unsigned >   _which_tree_used ;
};

template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   class iterator
   {
      friend class HashMap<HashKey, HashData>;
   public:
      iterator(vector<HashNode>* n = 0 , const size_t& s = 0 , size_t i = 0, size_t j = 0)
      :  _vp(n) , _size(s), _i(i) , _j(j) {  }
      iterator(){}
      ~iterator() {}
      const HashNode& operator * () const { return _vp[_i][_j] ; }
      iterator& operator ++ () {
         _j++;
         if( _j < _vp[ _i ].size() ) return(*this);
         else
         {
            _i ++ ;
            if( _i == _size ) { _j = 0 ; return (*this); } // end
            while( _vp[ _i].empty())  // next col is empty
            {
               _i++;
               if( _i == _size ) { _j = 0 ; return (*this); }
            }
            _j = 0;
            return (*this) ;
         }
      }   //  ++ k
      iterator& operator = (const iterator& i) {
         _vp = i._vp;
         _size = i._size;
         _i = i._i;
         _j = i._j; 
         return (*this); 
      }
      bool operator == (const iterator& i) const { 
         if( _vp == i._vp) 
         {
            if( _i == i._i && _j == i._j )return true; 
         }
          return false; 
      }
      bool operator != (const iterator& i) const { 
         if( *this == i )return false; else return true; 
      }
   private:
      std::vector<HashNode>* _vp ;
      size_t _size;
      size_t _i ;
      size_t _j ;
  
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // Point to the first valid data
   iterator begin() const 
   { 
      size_t i = 0;
      while( _buckets[i].empty() ) 
      {
         i++;
         if( i == (_numBuckets) ) break;
      }
      return iterator( _buckets , _numBuckets , i , 0 ); 
      //return iterator(); 
   }
   // Pass the end
   iterator end() const 
   { 
      return iterator( _buckets , _numBuckets , _numBuckets , 0 );
      //return iterator(); 
   }
   // return true if no valid data
   bool empty() const 
   { 
      for( size_t i = 0 ; i < _numBuckets ; i++)
      {
         if( !_buckets[i]. empty() ) return false ;
      }
      return true ; 
   }
   // number of valid data
   size_t size() const 
   { 
      size_t s = 0; 
      for( size_t i = 0 ; i < _numBuckets ; i++)
      {
        //cout << _buckets[i].size() << endl;
         s += _buckets[i]. size() ;
      }
      return s;  
   }

   void push( const HashKey& k , const unsigned& tree_id ) 
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            _buckets[n][i].second.push( tree_id ) ; // st_info
            return ;
         }
      }
      HashData d( tree_id ) ; 
      _buckets[n].push_back( HashNode( k , d ) ); // first time
   }
   bool erase( const HashKey& k , const unsigned& tree_id ) 
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.erase( tree_id ) ; // st_info
         }
      }
      assert(0) ;
   }
   unsigned get_last( const HashKey& k )
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.get_last() ; // st_info
         }
      }
   }
   vector< unsigned >& get_used_after( const Key& k )
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.get_used_after( ) ; // st_info
         }
      }
   }
   vector< unsigned >& get_used_before( const Key& k )
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.get_used_before( ) ; // st_info
         }
      }
   }
   vector< unsigned >& get_same_node( const Key& k )
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.get_same_node( ) ; // st_info
         }
      }
   }
   vector< unsigned >& get_tree_used( const Key& k )
   {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _buckets[n].size(); i++ )
      {
         if( k == _buckets[n][i].first ) 
         {
            return _buckets[n][i].second.get_tree_used( ) ; // st_info
         }
      }
   }

   void printallbuckets()
   {
      for( size_t i = 0 ; i < _numBuckets ; i++)
      cout << i << " : "<< _buckets[i].size() << endl;
   }
private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return ( k() % _numBuckets ); }

};


