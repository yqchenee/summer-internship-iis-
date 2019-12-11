// from relation fileA ( 1 -> 2 )
// id1 id2
// and  ids fileB 
// id1 
// insert (1) ids from relation file to ids file
// output to file name : xxids

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std ;

int main( int argc, const char** argv ) 
{
    string infile_rel = "../data/200k/" ;
    infile_rel.append( argv[1] );

    string infile_ids = "../data/200k/" ;
    infile_ids.append( argv[2] );

    string outfile = "../data/200k/" ;
    outfile.append( argv[3] ) ;

    fstream fin_rel( infile_rel.c_str() ) ;
    fstream fin_ids( infile_ids.c_str() ) ;
    fstream fout( outfile.c_str() ) ;
    
    if( !fin_rel.is_open() ) {
        cout << infile_rel << " is not exist." << endl;
        return 0 ;
    }
    if( !fin_ids.is_open() ) {
        cout << infile_ids << " is not exist." << endl;
        return 0 ;
    }

    if( !fout.is_open() ) {
        fout.open( outfile.c_str() , fstream::out ) ;
        cout << outfile << " is not exist." << endl;
        // return 0 ;
    }

    unsigned a , b ;
    vector<unsigned>* hash = new vector<unsigned> [10000] ;
    vector<unsigned> output ;

    while( fin_rel >> a >> b )
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
            output.push_back( a ) ;
            // fout << a << endl ;
        }
    }
    while( fin_ids >> a )
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
            output.push_back( a ) ;
            // fout << a << endl ;
        }
    }

    // sort
    sort( output.begin() , output.end() ) ;
    for( unsigned i = 0 ; i < output.size() ; i++ )
    {
        fout << output[i] << endl;
    }
}
