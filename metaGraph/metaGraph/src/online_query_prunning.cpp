#include "util.h"
#include "metaGraph.h"

#include <iostream>
#include <vector>
#include <time.h>

using namespace std;

int main(int argc, const char** argv) 
{
    if( argc < 3 ) {
        cout << "Usage: Run <string infoFile> <string offlineFile> <string queryFileS>" << endl;
        return 0 ;
    }

    string infile = argv[1] ;

    metaGraph MG ;
    if( !MG.read_info( infile ) ) {
        cout << infile << " is not existed." << endl;
        return 0 ;
    } 
    
    string   offline_Sim = argv[2] ;

    for( int i = 3 ; i < argc ; i++ ) {
        int Time ;
        unsigned Q_user , topK ;
        string   businessFile , outFile ;
        float threshold_dist = 0 ;

        string queryFile = "query_input/" ;
        queryFile.append( argv[i] ) ;
        if( !MG.readQueryFile( queryFile , Time , topK , Q_user , businessFile , outFile , threshold_dist )) 
            continue ;

        // input businessFile
        vector<unsigned> given_business ;
        if( !inputFile_to_vector( businessFile , given_business ) ) continue ;
        
        if( i == 3 ) {
            const clock_t begin_time = clock();
            if( !MG.read_offline_simularity( offline_Sim ) ) continue ; // offline_sim = "offline/" + offline_sim
            const clock_t end_time = clock();
            cout << "read offline simularity matrix costs " << float( end_time - begin_time ) / float(CLOCKS_PER_SEC) << "s" << endl;
        }
        
        clock_t filter_begin = clock() ;
        MG.reset_mask() ;
        if( Time != -1 ) MG.count_mask( (unsigned)Time ) ;
        clock_t filter_end = clock() ;
        cout <<  "calculate filter costs " << float( filter_end - filter_begin ) / float( CLOCKS_PER_SEC ) << "s" << endl;

        clock_t query_begin = clock() ;
        outFile = "query_result/" + outFile ;
        MG.online_Query_Prunning( outFile , given_business , Q_user , topK , threshold_dist ) ;
        clock_t query_end = clock() ;
        
        cout << "query costs " << float( query_end - query_begin ) / float(CLOCKS_PER_SEC) << "s" << endl;
        cout << "====================================================" << '\n' << endl;
    }
    
}