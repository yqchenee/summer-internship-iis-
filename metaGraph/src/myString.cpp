/****************************************************************************
  FileName     [ myString.cpp ]
  PackageName  [ util ]
  Synopsis     [ Customized string processing functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <string>
#include <ctype.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;

// 1. strlen(s1) must >= n
// 2. The first n characters of s2 are mandatory, they must be case-
//    insensitively compared to s1. Return less or greater than 0 if unequal.
// 3. The rest of s2 are optional. Return 0 if EOF of s2 is encountered.
//    Otherwise, perform case-insensitive comparison until non-equal result
//    presents.
//
bool inputFile_to_vector( string inputFile , vector<unsigned>& ret_vec ) {
    inputFile = "query_input/" + inputFile ;
    fstream inF( inputFile.c_str() ) ;
    if( !inF.is_open() ) {
        cout << inputFile << " is not existed." << endl ;
        return false ;
    }
    unsigned buf ;
    while( inF >> buf ) {
        ret_vec.push_back( buf ) ;
    }
    inF.close() ;
    if( ret_vec.empty() ) {
        cout << inputFile << " contains nothing." << endl;
        return false ;
    }
    else return true ;
}

int
myStrNCmp(const string& s1, const string& s2, unsigned n)
{
   assert(n > 0);
   unsigned n2 = s2.size();
   if (n2 == 0) return -1;
   unsigned n1 = s1.size();
   assert(n1 >= n);
   for (unsigned i = 0; i < n1; ++i) {
      if (i == n2)
         return (i < n)? 1 : 0;
      char ch1 = (isupper(s1[i]))? tolower(s1[i]) : s1[i];
      char ch2 = (isupper(s2[i]))? tolower(s2[i]) : s2[i];
      if (ch1 != ch2)
         return (ch1 - ch2);
   }
   return (n1 - n2);
}


// Parse the string "str" for the token "tok", beginning at position "pos",
// with delimiter "del". The leading "del" will be skipped.
// Return "string::npos" if not found. Return the past to the end of "tok"
// (i.e. "del" or string::npos) if found.
//
size_t
myStrGetTok(const string& str, vector<string>& tok, size_t pos = 0,
            const char del = ' ')
{
   tok.clear() ;
   string s = "\t" ;
   s.insert( 0 , 1 , del ) ; // s = del + \t

   while( true ) { 
      pos = str.find_first_not_of( s.c_str() , pos);
      if( pos == string::npos ) break;
      size_t end = str.find_first_of( s.c_str() , pos );
      tok.push_back( str.substr( pos , end - pos ) );
      pos = end ;
   }  
   return tok.size() ; 
}


// Convert string "str" to integer "num". Return false if str does not appear
// to be a number
bool
myStr2Int(const string& str, int& num)
{
   num = 0;
   size_t i = 0;
   int sign = 1;
   if (str[0] == '-') { sign = -1; i = 1; }
   bool valid = false;
   for (; i < str.size(); ++i) {
      if (isdigit(str[i])) {
         num *= 10;
         num += int(str[i] - '0');
         valid = true;
      }
      else return false;
   }
   num *= sign;
   return valid;
}

bool 
myStr2Unsigned(const string& str, unsigned& num)
{
   num = 0;
   size_t i = 0;
   bool valid = false;
   for (; i < str.size(); ++i) {
        if (isdigit(str[i])) {
            num *= 10;
            num += int(str[i] - '0');
            valid = true;
        }
        else return false;
   }
   return valid;
}
bool
myStr2Float(const string& str, float& num)
{
   num = 0;
   size_t i = 0;
   int sign = 1;
   if (str[0] == '-') { sign = -1; i = 1; }
   bool valid = false ;
   float dot  = 0 ;
   for (; i < str.size(); ++i) {
        if (isdigit(str[i])) {
            if( dot != 0 ){
                num += dot * float( str[i] - '0' ) ;
                dot *= 0.1 ;
                valid = true ;
            }
            else {
                num *= 10;
                num += int(str[i] - '0');
                valid = true;
            }
        }
        else if( str[i] == '.' ){
            if( dot != 0 ) return false ;
            dot = 0.1 ;
        }
        else return false;
   }
   num *= sign;
   return valid;
}

// Valid var name is ---
// 1. starts with [a-zA-Z_]
// 2. others, can only be [a-zA-Z0-9_]
// return false if not a var name
bool
isValidVarName(const string& str)
{
   size_t n = str.size();
   if (n == 0) return false;
   if (!isalpha(str[0]) && str[0] != '_')
      return false;
   for (size_t i = 1; i < n; ++i)
      if (!isalnum(str[i]) && str[i] != '_')
         return false;
   return true;
}

#include <math.h>
float
countRealDistance( const float& lng1 , const float& la1 , const float& lng2 , const float& la2 )
{
    return sqrt( (lng1-lng2)*(lng1-lng2) + (la1-la2)*(la1-la2) ) ;
}
float
countRealDistance( const pair<float , float>& p1 , const pair<float , float>& p2 )
{
    return countRealDistance( p1.first , p1.second , p2.first , p2.second ) ;
}