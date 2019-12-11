// input relation File
// id1 id2 
// output it with random time in range of ( minTime to maxTime )
// id1 id2 ts te

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <time.h>

#define minTime 0
#define maxTime 100

using namespace std ;

int main( int argc, const char** argv ) 
{
    string infile_rel = "../data/200k/" ;
    infile_rel.append( argv[1] );

    string outfile = "../data/200k/" ;
    outfile.append( argv[2] ) ;

    fstream fin_rel( infile_rel.c_str() ) ;
    fstream fout( outfile.c_str() ) ;
    
    if( !fin_rel.is_open() ) {
        cout << infile_rel << " is not exist." << endl;
        return 0 ;
    }

    if( !fout.is_open() ) {
        fout.open( outfile.c_str() , fstream::out ) ;
        cout << outfile << " is not exist." << endl;
        // return 0 ;
    }
    srand(time(NULL) ) ;
    unsigned a , b ;
    unsigned ts , te ;

    while( fin_rel >> a >> b ) {
        ts = rand() % 100 ;
        te = ts + ( rand() % (100-ts) ) ;
        fout << a << ' ' << b << ' ' << ts << ' ' << te << '\n' ;    
    }
}