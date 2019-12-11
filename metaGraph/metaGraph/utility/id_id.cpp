// from two ids file A,B
// output ids file C that
// contains id from A and B

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std ;

bool func( const unsigned& i , const unsigned& j ) {
    return ( i < j ) ;
}

int main( int argc, const char** argv ) 
{
    string infile_A = "../data/200k/" ;
    infile_A.append( argv[1] );
    string infile_B = "../data/200k/" ;
    infile_B.append( argv[2] );
    string outfile = "../data/200k/" ;
    outfile.append( argv[3] ) ;

    fstream fin_A( infile_A.c_str() ) ;
    fstream fin_B( infile_B.c_str() ) ;
    fstream fout( outfile.c_str() ) ;
    
    if( !fin_A.is_open() ) {
        cout << infile_A << " is not exist." << endl;
        return 0 ;
    }
    if( !fin_B.is_open() ) {
        cout << infile_B << " is not exist." << endl;
        return 0 ;
    }
    if( !fout.is_open() ) {
        fout.open( outfile.c_str() , fstream::out ) ;
    }

    vector<unsigned> vec_A ;
    vector<unsigned> vec_B ;
    unsigned id ;
    while( fin_A >> id ) {
        vec_A.push_back(id) ;
    }
    while( fin_B >> id ) {
        vec_B.push_back(id) ;
    }
    sort( vec_A.begin() , vec_A.end() , func ) ;
    sort( vec_B.begin() , vec_B.end() , func ) ;

    unsigned i = 0 , j = 0 ;
    while( i < vec_A.size() && j < vec_B.size() ) {
        if( vec_A[i] < vec_B[j] ) {
            fout << vec_A[i] << '\n' ;
            i++ ;
        }
        else if ( vec_A[i] > vec_B[j] )
        {
            fout << vec_B[j] << '\n' ;
            j++ ;
        }
        else // == 
        {
            fout << vec_A[i] << '\n' ;
            i++ ;
            j++ ;
        }
    }
    for( ; i < vec_A.size() ; i++ ) fout << vec_A[i] << '\n' ;
    for( ; j < vec_B.size() ; j++ ) fout << vec_B[j] << '\n' ;

}