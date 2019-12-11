#include "util.h"
#include "metaGraph.h"

#include <iostream>
#include <vector>
#include <time.h>

using namespace std;

int main(int argc, const char** argv) 
{
    if( argc < 2 ) {
        cout << "Usage: Run <string infoFile> <string queryFileS>" << endl;
        return 0 ;
    }

    string infile = argv[1] ;

    metaGraph MG ;
    if( !MG.read_info( infile ) ) {
        cout << infile << " is not existed." << endl;
        return 0 ;
    } 
    
    for( int i = 2 ; i < argc ; i++ ) {
        unsigned topK ;
        string   outfile ;

        string queryFile = "query_input/" ;
        queryFile.append( argv[i] ) ;
        if( !MG.readQueryFile( queryFile , topK , outfile ) )
            continue ;

        if( i == 2 ) {
            const clock_t begin_time = clock();
            MG.main_counting() ;
            const clock_t end_time = clock();
            cout << "calculate matrix costs " << float( end_time - begin_time ) / float(CLOCKS_PER_SEC) << "s" << endl;
        }

        clock_t query_begin = clock() ;
        outfile = "query_result/" + outfile ;
        MG.output_final_matrix( outfile , topK ) ; // with mask = 1 
        clock_t query_end = clock() ;
        
        cout << "query matrix costs " << float( query_end - query_begin ) / float(CLOCKS_PER_SEC) << "s" << endl;
        cout << "====================================================" << '\n' << endl;
    }
    
}