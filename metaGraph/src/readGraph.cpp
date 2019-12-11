
#include "metaGraph.h"

#include <iostream>
#include <fstream>

using namespace std ;

void error_input( const string& filename ) {
    cout << "input relation is error : " << filename << endl;
    assert(0) ;
} 

bool metaGraph::read_info( const string& infile ) {
    ifstream fin( infile.c_str() ) ; // info_file in
    if( !fin.is_open() ) {
        return false ;
    }

    unsigned numbers = 0 ;
    
    // input id file 
    fin >> numbers ;
    _num_ids = numbers ;
    for( unsigned i = 0 ; i < _num_ids ; i++ ) 
    {
        string      type_id ;
        char        comma ;
        string      file_path ;
        fin >> type_id >> comma >> file_path ;
        readGraph_id( file_path , type_id ) ;
    }

    // input relation file
    fin >> numbers ;
    _num_rel = numbers ;
    for( unsigned i = 0 ; i < _num_rel ; i++ ) 
    {
        string      type_a , type_b ;
        string      file_path ;
        fin >> type_a >> type_b >> file_path ;
        readGraph_rel( file_path , type_a , type_b ) ;
    }
    
    //input level 
    fin >> numbers ;
    _num_level = numbers ; 
    if( !read_topology( fin ) ) error_input( infile ) ;

    return true ;
}

// read id_file and construct dict for type_id
void metaGraph::readGraph_id( const string& infile , const string& type_id ) 
{
    string filename = "data/" + infile ;

    fstream fin( filename.c_str() ) ;    
    if( !fin.is_open() ) {
        cout << filename << " is not exist." << endl;
        return ;
    }
    
    string str ;
    vector<unsigned_Key> v_uk_id ; // vector_Unsigned_Key_id
    vector< pair< float , float> > position ;

    while( getline( fin , str ) ){ // str : id , latitude , longtitude
        vector<string> tokens ;
        unsigned id ;
        float la = 0 , lo = 0 ;
        size_t len = myStrGetTok( str , tokens , 0 , ' ' ) ;
        
        switch( len ) {
            // id
            case 1 : 
                break ;
            // id , lo , la
            case 3 : 
                if( !myStr2Float( tokens[1] , lo ) ) error_input( filename ) ;
                if( !myStr2Float( tokens[2] , la ) ) error_input( filename ) ;
                break ;
            default :
                error_input( filename ) ;
        }

        if( !myStr2Unsigned( tokens[0] , id )) error_input( filename ) ;

        unsigned_Key* k = new unsigned_Key(id) ;
        v_uk_id.push_back( *k ) ;
        pair< float , float > pos( lo , la ) ;
        position.push_back( pos ) ;
    }

    assert( v_uk_id.size() == position.size() ) ;

    dict< unsigned_Key , id_info>* id_m = new dict< unsigned_Key , id_info> ( v_uk_id.size() ) ;
    dict< unsigned_Key , unsigned>* r_id_m = new dict< unsigned_Key , unsigned> ( v_uk_id.size() ) ;
    for( unsigned i = 0 ; i < v_uk_id.size() ; i++ ) {
        unsigned_Key* reverse_map_key = new unsigned_Key(i) ;
        r_id_m-> write( *reverse_map_key , v_uk_id[i]() ) ;
        id_info* info = new id_info( i , position[i].first , position[i].second ) ;
        id_m-> write( v_uk_id[i] , *info ) ;
    }

    _idMapping.push_back( id_m ) ;
    _r_idMapping.push_back( r_id_m ) ;
    _index_to_type_id.push_back ( type_id ) ;
}

void metaGraph::readGraph_rel( const string& infile , const string& type_a , const string& type_b )
{
    string filename = "data/" + infile ;
    cout << "read relation " << type_a << ' ' << type_b << endl;
    fstream fin( filename.c_str() ) ;    
    if( !fin.is_open() ) {
        cout << filename << " is not exist." << endl;
        return ;
    }

    // use input type_a type_b to find idMapping-dictionary
    unsigned idMapping_index_a = find_dict( type_a ) ;
    unsigned idMapping_index_b = find_dict( type_b ) ;
    const dict< unsigned_Key , id_info>* const dict_a = _idMapping[ idMapping_index_a ] ;
    const dict< unsigned_Key , id_info>* const dict_b = _idMapping[ idMapping_index_b ] ;

    sparse_matrix* spm = new sparse_matrix( dict_a-> size() , dict_b-> size() ) ;
    
    string str ;
    while( getline( fin , str ) ){ // str : id1 , id2 , ts , te , weight
        vector<string> tokens ;

        int id_a , id_b , weight = 1 , ts = 0 , te = UINT_MAX ;
        size_t len = myStrGetTok( str , tokens , 0 , ' ' ) ;
        
        switch( len ) {
            // str : id1 , id2
            case 2 : break ;
            // str : id1 , id2 , weight
            case 3 : if( !myStr2Int( tokens[2] , weight ) ) error_input( filename ) ; break ;
            // str : id1 , id2 , ts , te 
            case 4 : 
                if( !myStr2Int( tokens[2] , ts ) ) error_input( filename ) ;
                if( !myStr2Int( tokens[3] , te ) ) error_input( filename ) ;
                break; 
            // str : id1 , id2 , ts , te , weight
            case 5 : 
                if( !myStr2Int( tokens[2] , ts ) ) error_input( filename ) ;
                if( !myStr2Int( tokens[3] , te ) ) error_input( filename ) ;
                if( !myStr2Int( tokens[4] , weight ) ) error_input( filename ) ;
                break;
            default : error_input( filename ) ;
        }

        if( !myStr2Int( tokens[0] , id_a ) ) error_input( filename ) ;
        if( !myStr2Int( tokens[1] , id_b ) ) error_input( filename ) ;
        
        unsigned_Key* key_a = new unsigned_Key(id_a) ; // which row
        unsigned_Key* key_b = new unsigned_Key(id_b) ; // which col
        sp_m_ele* ele = new sp_m_ele( (*dict_b)[ *key_b ]._ind , ts , te , weight ) ;
        
        spm -> insert_ele( (*dict_a)[ *key_a ]._ind , *ele ) ;
        
    }

    relation_matrix* rel_m = new relation_matrix( spm , type_a , type_b ) ;
    _rel_matrix.push_back( *rel_m ) ;
}

bool metaGraph::read_topology( ifstream& fin ) 
{
    // 0 : 1_0 
    // node assignment 
    string str ;
    unsigned count = 0 ;
    getline( fin , str ) ; // remain line 
    _level_node_index_in_nodeL = new vector< unsigned > [_num_level ] ; 

    while( getline( fin , str ) ) { 
        vector<string> tokens ;
        size_t len = myStrGetTok( str , tokens , 0 , ' ' ) ;
        
        unsigned n ; 
        if( !myStr2Unsigned( tokens[0] , n ) ) return false ; // 0
        if( n != count ) return false ;
        if( tokens[1].compare( ":" ) != 0 ) return false ; // :

        for( unsigned i = 2 ; i < len ; i++ ) // 1_0
        {
            vector<string> type_and_id ;
            size_t l = myStrGetTok( tokens[i] , type_and_id , 0 , '_') ;
            if( l != 2 ) return false ;
            int id ;
            if( !myStr2Int( type_and_id[1] , id ) ) return false ;
            
            node* n = new node( type_and_id[0] , id , count ) ;
            _nodeList.push_back( n ) ;
            _level_node_index_in_nodeL[ count ].push_back( _nodeList.size()-1 ) ;
        }

        if( ++count == _num_level ) break;
    }

    getline( fin , str ) ; // '\n'

    // id1 id2
    // path assignment : id1 -> id2
    unsigned id1 , id2 ;
    while( fin >> id1 >> id2 ) { 
        node* n1 = 0 ;
        node* n2 = 0 ;
        // find n1 and n2
        for( unsigned i = 0 ; i < _nodeList.size() ; i++ )
        {
            if( _nodeList[i]-> _id == id1 ) {
                if( n1 != 0 ) {
                    cout << "more than one node have id " << id1 << endl;
                    return false ;
                }
                n1 = _nodeList[i] ; 
            }
            else if( _nodeList[i]-> _id == id2 ) {
                if( n2 != 0 ) {
                    cout << "more than one node have id " << id2 << endl;
                    return false ;
                }
                n2 = _nodeList[i] ;
            }
        }
        n1 -> push_child( n2 ) ;
        n2 -> push_parent( n1 ) ;
    }
    return true ;
}
// main
bool metaGraph::readQueryFile( const string& qF , unsigned& topK , string& outfile ) {
    ifstream in( qF.c_str() ) ;
    if( !in.is_open() ) {
        cout << qF << " is not exist." << endl;
        return false ;
    }
    in >> topK >> outfile ;
    return true ;
}
// BQT
bool metaGraph::readQueryFile( const string& qF , int& Time , unsigned& topK , unsigned& user , 
                            string& businessFile , string& outfile , float& threshold_dist  )
{
    ifstream in( qF.c_str() ) ;
    if( !in.is_open() ) {
        cout << qF << " is not exist." << endl;
        return false ;
    }
    in >> Time >> topK >> user >> businessFile >> outfile ;
    
    if( in >> threshold_dist ) ;
    else threshold_dist = 0 ;

    return true ;
}
// BQT_KU
bool metaGraph::readQueryFile( const string& qF , int& Time , unsigned& user , string& businessFile , 
                            string& k_user_File , string& outfile , float& threshold_dist )
{
    ifstream in( qF.c_str() ) ;
    if( !in.is_open() ) {
        cout << qF << " is not exist." << endl;
        return false ;
    }
    in >> Time >> user >> businessFile >> k_user_File >> outfile ;

    if( in >> threshold_dist ) ;
    else threshold_dist = 0 ;

    return true ;
}

// construct metaGraph::_offline_sim_matrix
bool metaGraph::read_offline_simularity( string offline_Sim )
{
    string dir = "offline/query_result/" ;
    dir.append( offline_Sim ) ;

    ifstream in( dir.c_str() ) ;
    if( !in.is_open() ) {
        cout << dir << " is not exist." << endl;
        return false ;
    }
    if( !_offline_sim_matrix ) {
        delete _offline_sim_matrix ;
        _offline_sim_matrix = 0 ;
    }

    if( _level_node_index_in_nodeL ) {  // topology is construct 
        const unsigned& first       = _level_node_index_in_nodeL[ 0 ][0] ;
        const unsigned& last        = _level_node_index_in_nodeL[ _num_level-1 ][0] ;
        const node* const first_node = _nodeList[ first ] ;
        const node* const last_node  = _nodeList[ last ] ;
        string first_type = first_node-> _type ;
        string last_type  = last_node-> _type ;
        unsigned idMapping_index_a = find_dict( first_type ) ;
        unsigned idMapping_index_b = find_dict( last_type ) ;
        const dict< unsigned_Key , id_info>* const dict_a = _idMapping[ idMapping_index_a ] ;
        const dict< unsigned_Key , id_info>* const dict_b = _idMapping[ idMapping_index_b ] ;

        _offline_sim_matrix = new sparse_matrix( dict_a-> size() , dict_b-> size() ) ;

        unsigned id_a , id_b ;
        int weight ;
        while( in >> id_a >> id_b >> weight ) {
            unsigned_Key* key_a = new unsigned_Key(id_a) ; // which row
            unsigned_Key* key_b = new unsigned_Key(id_b) ; // which col
            sp_m_ele* ele = new sp_m_ele( (*dict_b)[ *key_b ]._ind , weight ) ;
            
            _offline_sim_matrix -> insert_ele( (*dict_a)[ *key_a ]._ind , *ele ) ;
        }
        return true ;
    }
    else {
        cout << "topology is not exist yet." << endl;
        return false ;
    }
}