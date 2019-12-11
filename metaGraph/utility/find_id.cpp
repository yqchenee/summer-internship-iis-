// from relation file ( 1 -> 2 ) 
// id1 id2
// generate (2) ids

#include <vector>
#include <iostream>
#include <fstream>

using namespace std ;

int main( int argc, const char** argv ) 
{
    string infile = "../data/200k/" ;
    infile.append( argv[1] );

    string outfile = "../data/200k/" ;
    outfile.append( argv[2] ) ;

    fstream fin( infile.c_str() ) ;
    fstream fout( outfile.c_str() ) ;
    
    if( !fin.is_open() ) {
        cout << infile << " is not exist." << endl;
        return 0 ;
    }
    if( !fout.is_open() ) {
        fout.open( outfile.c_str() , fstream::out ) ;
        cout << outfile << " is not exist." << endl;
        // return 0 ;
    }

    unsigned a , b ;
    vector<unsigned>* hash = new vector<unsigned> [10000] ;

    while( fin >> a >> b )
    {
        // cout << a << b << endl;
        unsigned key = ( a % 10000 ) ;    
        bool find = false ;
        for( unsigned i = 0 ; i < hash[key].size() ; i++ )
        {
            if( a == hash[key][i] ) {
                find = true ;
                break; 
            }
        }
        if( !find ) {
            hash[key].push_back( a ) ;
            fout << a << endl ;
        }
    }
}
