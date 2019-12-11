#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <iostream>
#include <cassert>
#include <utility> 
#include <cfloat>

using namespace std;

// In myString.cpp
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, vector<string>& tok, size_t pos = 0,
                    const char del = ' ') ;
extern bool myStr2Int(const string& str, int& num);
extern bool myStr2Float(const string& str, float& num) ;
extern bool myStr2Unsigned(const string& str, unsigned& num) ;
extern bool isValidVarName(const string& str);

extern bool inputFile_to_vector( string inputFile , vector<unsigned>& ret_vec ) ;
extern float countRealDistance( const float& lng1 , const float& la1 , const float& lng2 , const float& la2 ) ;
extern float countRealDistance( const pair<float , float>& p1 , const pair<float , float>& p2 ) ;

class unsigned_Key
{
public:
   unsigned_Key( const unsigned& k ) : _k(k) {}
   size_t operator() () const { return _k ;} 
   bool operator == ( const unsigned_Key& k ) const
   {
      if( k._k == _k ) return true;
      return false;
   }
   /*friend iostream& operator << (ostream& os , const unsigned_Key& k ) {
      return os << k._k ;
   }*/
private:
   unsigned    _k ;
};

template <class d_Key , class d_Data>
class dict
{
typedef pair<d_Key, d_Data> d_Node ;

public:
   dict() : _numBuckets(0) , _dict(0) {}
   dict(size_t s) : _numBuckets(0) , _dict(0) { init(s); }
   dict( const vector< d_Key >& v_id ) : _numBuckets(0) , _dict(0)
   { 
      init( v_id.size() );
      for( unsigned i = 0 ; i < v_id.size() ; i++ ) {
         write( v_id[i] , i ) ;
      }
   }
   ~dict() { reset(); }

   void reset() {  _numBuckets = 0;  if (_dict) { delete [] _dict; _dict = 0; } }
   void init(size_t s) { 
      reset(); 
      _numBuckets = s; 
      _dict = new vector<d_Node>[s]; 
   }

   size_t numBuckets() const { return _numBuckets; }

   vector<d_Node>& operator [] (size_t i) { return _dict[i]; }
   const vector<d_Node>& operator [](size_t i) const { return _dict[i]; }

   d_Data& operator [] ( d_Key& k) { return query(k) ; }
   const d_Data& operator [] ( d_Key& k) const { return query(k) ; }

   const d_Data& get( d_Key& k ) const {
      return query(k) ;
   }
   
   size_t size() const 
   { 
      size_t s = 0; 
      for( size_t i = 0 ; i < _numBuckets ; i++)
      {
        //cout << _dict[i].size() << endl;
         s += _dict[i]. size() ;
      }
      return s;  
   }

   // return false if dict miss
   bool read(const d_Key& k, d_Data& d) const {
      size_t n = bucketNum( k ) ;
      for( size_t i = 0 ; i < _dict[n].size(); i++ )
      {
         if( k == _dict[n][i].first ) 
         {
            d = _dict[n][i].second ;
            return true ;
         }
      }
      return false ;
   }
   // If k is already in the dict, overwrite the dictData
   void write(const d_Key& k, const d_Data& d) {
      size_t n = bucketNum( k ) ;
      _dict[n].push_back( d_Node( k , d ) ) ;
   }
   
   d_Data& query(const d_Key& k ) const 
   { 
      size_t n = bucketNum( k ) ; // find bucket
      for( size_t i = 0 ; i < ( _dict[n].size() ) ; i++ )
      {
         if( k == _dict[n][i].first ) 
         {
            return _dict[n][i].second ;
         }
      }
      cout << "Error : Key " << k() << " is not existed yet." << endl;
      assert(0) ;
   }
   
   bool find( const d_Key& k ) const
   {
      size_t n = bucketNum( k ) ; // find bucket
      for( size_t i = 0 ; i < ( _dict[n].size() ) ; i++ )
      {
         if( k == _dict[n][i].first ) 
         {
            return true ;
         }
      }
      return false ;
   }
   void printallbuckets()
   {
      for( size_t i = 0 ; i < _numBuckets ; i++)
      cout << i << " : "<< _dict[i].size() << endl;
   }


private:
   size_t            _numBuckets ;
   vector<d_Node>*   _dict ;

   size_t bucketNum(const d_Key& k) const {
      return (k() % _numBuckets); }
};


#endif // UTIL_H