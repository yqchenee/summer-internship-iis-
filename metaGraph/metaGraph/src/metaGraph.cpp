#include "metaGraph.h"
#include "heap.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
using namespace std ;

void transform_id_to_ind( const vector<unsigned>& id , vector<unsigned>& mat_ind , vector< pair< float , float>>& pos , \
                    dict< unsigned_Key , id_info>* mapping )
{
    for( unsigned i = 0 ; i < id.size() ; i++ ) {
        unsigned_Key* Key = new unsigned_Key( id[i] ) ;
        const id_info& info = (*mapping)[ *Key ] ;
        mat_ind.push_back( info._ind );
        pair< float , float > b_p ( info._longtitude , info._latitude ) ;
        pos.push_back( b_p ) ;
        delete Key ;
    }
}

void metaGraph::main_counting() // time = -1 -> no query
{
    // ordered by level
    for( unsigned lev = 0 ; lev < _num_level ; lev++ ) 
    {
        const vector<unsigned>& v_level = _level_node_index_in_nodeL[lev] ;
        //cout << "level: " << lev << endl;
        for( unsigned ll = 0 ; ll < v_level.size() ; ll++ ) 
        {
            node* now_node = _nodeList[ v_level[ll] ] ;
            if( now_node-> _childs.size() >= 2 ) { // more than two child
                for( unsigned c = 0 ; c < now_node-> _childs.size() ; c++ )
                {   // let child _last_split be self
                    now_node-> _childs[c] -> _last_split = now_node ;
                }
            }
            if( now_node-> _parents.size() == 1 ) {
                node* parent = now_node-> _parents[0] ;
                sparse_matrix* rel = find_relation_matrix( parent-> _type , now_node-> _type );
                
                // parent just split or not have matrix yet
                if( parent == now_node -> _last_split || !parent-> _rec_matrix ) { 
                    now_node-> _rec_matrix = rel ;
                }
                else {
                    sparse_matrix* spm = parent-> _rec_matrix -> dot( rel ) ;
                    now_node-> _rec_matrix = spm ;
                    now_node-> _last_split = parent-> _last_split ;
                }
            }
            else { // parent >= 2 , parent must have rec_matrix
                if( lev == 0 ) continue ;

                vector< sparse_matrix* > rec_matrixs ;
                node* l_Split = now_node -> _parents[0] -> _last_split ;
                for( unsigned i = 0 ; i < now_node->_parents.size() ; i++ ) 
                {
                    node* parent = now_node-> _parents[i] ;
                    
                    sparse_matrix* rel = find_relation_matrix( parent-> _type , now_node-> _type );
                    if( parent -> _rec_matrix ) {
                        sparse_matrix* spm = parent-> _rec_matrix -> dot( rel ) ;;
                        rec_matrixs.push_back( spm ) ;
                    }
                    else rec_matrixs.push_back( rel ) ; // parent not have matrix yet
                }

                sparse_matrix* rec = rec_matrixs[0] ;
                for( unsigned i = 1 ; i < rec_matrixs.size() ; i++ )
                {
                    rec = rec -> PairWise_dot( rec_matrixs[i] );
                }
                now_node -> _rec_matrix = rec ;
                now_node -> _last_split = l_Split-> _last_split ;

                if( l_Split -> _rec_matrix )  {
                    sparse_matrix* rec_last = l_Split-> _rec_matrix -> dot( rec ) ;
                    now_node -> _rec_matrix = rec_last ;
                }
            }
        }
    }
}

void error( string a , string b ) {
    cout << "not found relation " << a << " -> " << b << endl ; 
    assert(0) ;
}

sparse_matrix* metaGraph::find_relation_matrix( const string& type1 , const string& type2 ) {
    for( unsigned i = 0 ; i < _rel_matrix.size() ; i++ ) {
        const relation_matrix& rel_m = _rel_matrix[i] ;
        string t1 = rel_m.get_type1() ;
        string t2 = rel_m.get_type2() ;
        if( type1 == t1 && type2 == t2 ) return (rel_m.get_matrix()) ;
        else if ( type1 == t2 && type2 == t1) {
            return rel_m.get_matrix()-> transposed() ;
        }
    }

    error( type1 , type2 ) ;
    return NULL ;
}

void metaGraph::count_mask( const unsigned time )
{
    cout << "count mask for time " << time << endl ;
    reset_mask() ;
    for( unsigned lev = 0 ; lev < _num_level ; lev++ ) 
    {
        const vector<unsigned>& v_level = _level_node_index_in_nodeL[lev] ;
        for( unsigned ll = 0 ; ll < v_level.size() ; ll++ ) 
        {
            node* now_node = _nodeList[ v_level[ll] ] ;
            if( now_node-> _childs.size() >= 2 ) { 
                for( unsigned c = 0 ; c < now_node-> _childs.size() ; c++ )
                {   
                    now_node-> _childs[c] -> _last_split = now_node ;
                }
            }
            if( now_node-> _parents.size() == 1 ) {
                node* parent = now_node-> _parents[0] ;
                sparse_matrix* rel = find_relation_matrix( parent-> _type , now_node-> _type );

                rel = rel-> get_matrix_at_time_T( time ) ;

                if( parent == now_node -> _last_split || !parent-> _mask ) { 
                    now_node-> _mask = rel ;
                }
                else {
                    sparse_matrix* spm = parent-> _mask -> filter_dot( rel ) ;
                    now_node-> _mask = spm ;
                    now_node-> _last_split = parent-> _last_split ;
                }
            }
            else { 
                if( lev == 0 ) continue ;

                vector< sparse_matrix* > rec_matrixs ;
                node* l_Split = now_node -> _parents[0] -> _last_split ;
                for( unsigned i = 0 ; i < now_node->_parents.size() ; i++ ) 
                {
                    node* parent = now_node-> _parents[i] ;
                    
                    sparse_matrix* rel = find_relation_matrix( parent-> _type , now_node-> _type );

                    rel = rel-> get_matrix_at_time_T( time ) ;
                   
                    if( parent -> _mask ) {
                        sparse_matrix* spm = parent-> _mask -> filter_dot( rel ) ;;
                        rec_matrixs.push_back( spm ) ;
                    }
                    else rec_matrixs.push_back( rel ) ; // parent not have matrix yet
                }

                sparse_matrix* rec = rec_matrixs[0] ;
                for( unsigned i = 1 ; i < rec_matrixs.size() ; i++ )
                {
                    rec = rec -> filter_PairWise_dot( rec_matrixs[i] );
                }
                now_node -> _mask = rec ;
                now_node -> _last_split = l_Split-> _last_split ;

                if( l_Split -> _mask )  {
                    sparse_matrix* rec_last = l_Split-> _mask -> filter_dot( rec ) ;
                    now_node -> _mask = rec_last ;
                }
            }
        }
    }
}
// first
void metaGraph::output_final_matrix( const string& outfile , const unsigned topK ) {
    const unsigned& first       = _level_node_index_in_nodeL[ 0 ][0] ;
    const unsigned& last        = _level_node_index_in_nodeL[ _num_level-1 ][0] ;
    const node* const first_node = _nodeList[ first ] ;
    const node* const last_node  = _nodeList[ last ] ;
    string first_type = first_node-> _type ;
    string last_type  = last_node-> _type ;
    unsigned f_node_dict = find_dict( first_type ) ;
    unsigned l_node_dict = find_dict( last_type  ) ;

    sparse_matrix* spm_timeT = last_node-> _rec_matrix ;
    spm_timeT -> get_top_K_ele( topK , outfile , _r_idMapping[f_node_dict] , _r_idMapping[l_node_dict] ) ;
    
}

/************************************/
// second
/************************************/
void metaGraph::output_result_with_BQT( const string& outfile , const vector<unsigned>& given_business , 
                                        unsigned user , unsigned topK , float threshold_dist ) {
    const unsigned& first       = _level_node_index_in_nodeL[ 0 ][0] ;
    const unsigned& last        = _level_node_index_in_nodeL[ _num_level-1 ][0] ;
    const node* const first_node = _nodeList[ first ] ;
    const node* const last_node  = _nodeList[ last ] ;
    string first_type = first_node-> _type ;
    string last_type  = last_node-> _type ;
    unsigned f_node_dict = find_dict( first_type ) ;
    unsigned l_node_dict = find_dict( last_type ) ; // given business -> col

    // transform Business id to col 
    vector<unsigned> given_business_col ;
    vector< pair< float , float> > given_business_pos ;
    transform_id_to_ind( given_business , given_business_col , given_business_pos , _idMapping[l_node_dict] ) ;
    
    // transform User id to row
    unsigned_Key* Key = new unsigned_Key( user ) ;
    unsigned user_row = (*_idMapping[f_node_dict])[ *Key ]._ind;
    delete Key ;

    sparse_matrix* spm_timeT = _offline_sim_matrix ;
    if( last_node-> _mask ) 
        spm_timeT = spm_timeT-> PairWise_dot( last_node-> _mask , true ) ;

    vector<row_info> user_score ;
    // simularity score
    if( threshold_dist == 0 ) {
        spm_timeT -> calculate_SIM( given_business_col , user_score ) ;
    }
    else {
        vector< pair< float , float> > user_pos ;
        user_pos.resize( spm_timeT-> getNumRows() ) ;
        getUserPosition( _idMapping[f_node_dict] , _r_idMapping[f_node_dict] , user_pos ) ;

        spm_timeT -> calculate_SIM( given_business_col , given_business_pos , user_pos , threshold_dist , user_score ) ;
    }
    // social distance , assume the first node is user
    // so get the user-user matrix , just call find( first , first )
    find_relation_matrix( first_type , first_type ) -> calculate_socDist( user_row , user_score ) ;
    output_topK( outfile , user_score , topK , _r_idMapping[f_node_dict] ) ; // ind 2 id
}

// sort function
bool sort_score( const row_info& i , const row_info& j ) {
    if( i._socDist == 0 ) return false ;
    if( j._socDist == 0 ) return true ;
    float ii = float(i._simScore) / float(i._socDist) ;
    float jj = float(j._simScore) / float(j._socDist) ;
    return( ii > jj ) ;
}
void metaGraph::output_topK( const string& outfile , vector<row_info>& user_score ,\
                    unsigned topK , const dict< unsigned_Key , unsigned>* const idM ) 
{
    cout << "====================================================" << endl;
    fstream fout ;
    fout.open( outfile.c_str() , fstream::out ) ;

    cout << "get top ( " << topK << " ) elements." << endl;
    if( user_score.empty() ) {
        cout << "no positive score." << endl;
        return ;
    }

    sort( user_score.begin() , user_score.end() , sort_score ) ;
    if( user_score.back()._socDist == 0 ) user_score.pop_back() ; // pop the query user itself
    float total_score = 0 ;

    topK = ( topK > user_score.size()) ? user_score.size() : topK ;
    for( unsigned i = 0 ; i < topK ; i++ ) {
        unsigned_Key* Key = new unsigned_Key( user_score[i]._row ) ;
        float score = ( float( user_score[i]._simScore) / float(user_score[i]._socDist) ) ;
        total_score += score ;
        fout << (*idM)[ *Key ] << '\t' << setprecision(5) << score ;
        fout << setfill(' ') << setw(10) << setprecision(5) << total_score << '\n' ;
    }
    cout << "output at file : " << outfile << endl;
}

/************************************/
// third
/************************************/
bool metaGraph::calculate_final_query_score( const string& outfile , const vector<unsigned>& given_business , 
                            const vector<unsigned>& k_user , unsigned user , float threshold_dist )
{
    const unsigned& first       = _level_node_index_in_nodeL[ 0 ][ 0 ] ;
    const unsigned& last        = _level_node_index_in_nodeL[ _num_level-1 ][0] ;
    const node* const first_node = _nodeList[ first ] ;
    const node* const last_node  = _nodeList[ last ] ;
    string first_type = first_node-> _type ;
    string last_type  = last_node-> _type ;
    unsigned f_node_dict = find_dict( first_type ) ;
    unsigned l_node_dict = find_dict( last_type ) ; // given business -> col

    // transform Business id to col 
    vector<unsigned> given_business_col ;
    vector< pair< float , float> > given_business_pos ;
    transform_id_to_ind( given_business , given_business_col , given_business_pos , _idMapping[l_node_dict] ) ;
    
    // transform user id to row 
    vector<unsigned> k_user_row ;
    vector< pair< float , float> > k_user_pos ;
    transform_id_to_ind( k_user , k_user_row , k_user_pos , _idMapping[f_node_dict] ) ;

    // transform User id to row
    unsigned_Key* Key = new unsigned_Key( user ) ;
    unsigned User_row = (*_idMapping[f_node_dict])[ *Key ]._ind ;
    delete Key ;

    sparse_matrix* spm_timeT = _offline_sim_matrix ;
    if( last_node-> _mask )
        spm_timeT = spm_timeT-> PairWise_dot( last_node-> _mask , true ) ;

    vector<row_info> user_score ;
    // simularity score
    if( threshold_dist == 0 ) {
        spm_timeT -> calculate_SIM( given_business_col , user_score ) ;
    }
    else {
        vector< pair< float , float> > user_pos ;
        user_pos.resize( spm_timeT-> getNumRows() ) ;
        getUserPosition( _idMapping[f_node_dict] , _r_idMapping[f_node_dict] , user_pos ) ;

        spm_timeT -> calculate_SIM( given_business_col , given_business_pos , user_pos , threshold_dist , user_score ) ;
    }
    // social distance , assume the first node is user
    // so get the user-user matrix , just call find( first , first )
    find_relation_matrix( first_type , first_type ) -> calculate_socDist( User_row , user_score ) ;
    return calculate_query_k_user( outfile , user_score , k_user_row ) ;
}

// from user_score , if k_user in it , plus it to output score
bool metaGraph::calculate_query_k_user( const string& outfile , const vector<row_info>& user_score , const vector<unsigned>& k_user_row )
{
    cout << "====================================================" << endl;
    fstream fout ;
    fout.open( outfile.c_str() , ios::out ) ;
    
    if( user_score.empty() ) {
        cout << "No positive score amoung all users." << endl;
        fout << "No positive score amoung all users." << endl;
        return true ;
    }

    dict< unsigned_Key , unsigned > hash( user_score.size() ) ; // matrix_row_index -- user_score_vector_index
    for( unsigned i = 0 ; i < user_score.size() ; i++ ) {
        unsigned_Key* Key = new unsigned_Key( user_score[i]._row ) ;
        hash.write( *Key , i ) ;
        delete Key ;
    }

    float score = 0 ;
    for( unsigned i = 0 ; i < k_user_row.size() ; i++ ) {
        unsigned v_ind ;
        unsigned_Key* Key = new unsigned_Key( k_user_row[i] ) ;
        if( hash.read( *Key , v_ind ) ) {
            if( !user_score[v_ind]._socDist ) {
                // query-k_user contains given User itself
                return false ;
            }
            score += (float)user_score[ v_ind ]._simScore / (float)user_score[v_ind]._socDist ;
        }
        delete Key ;
    }
    fout << "query " << k_user_row.size() << " users , the final total score is: " << score << endl;
    
    cout << "query " << k_user_row.size() << " users , the final total score is: " << score << endl;
    return true ;
}

// 
void metaGraph::getUserPosition( const dict< unsigned_Key , id_info>* const _idM ,
                const dict< unsigned_Key , unsigned>* const r_idM , vector<pair <float , float>>& user_pos )
{
    for( unsigned i = 0 ; i < user_pos.size() ; i++ ) {
        unsigned_Key* uK = new unsigned_Key( i ) ;
        unsigned id = r_idM-> get( *uK ) ;
        delete uK ;
        uK = new unsigned_Key( id ) ;
        const id_info& info = _idM-> get( *uK ) ;
        pair< float , float > pos( info._longtitude , info._latitude ) ;
        user_pos[i] = pos ; 
    }
}

/************************************/
// forth
/************************************/
class id_state
{
public :
    id_state() { examined = false ; candidate = true ;}
    int  score ;
    bool examined ;
    bool candidate ;
};

bool sort_by_row( const row_info& a , const row_info& b ) {
    return ( a._row < b._row ) ;
}
bool sort_by_score( const row_info& a , const row_info& b ) {
    return ( a._simScore > b._simScore ) ;
}
bool metaGraph::online_Query_Prunning( const string& outfile , const vector<unsigned>& given_business , 
                                unsigned user , unsigned topK , float threshold_dist )
{
    typedef pair< float , unsigned > heap_ele ;
    const unsigned& first       = _level_node_index_in_nodeL[ 0 ][0] ;
    const unsigned& last        = _level_node_index_in_nodeL[ _num_level-1 ][0] ;
    const node* const first_node = _nodeList[ first ] ;
    const node* const last_node  = _nodeList[ last ] ;
    string first_type = first_node-> _type ;
    string last_type  = last_node-> _type ;
    unsigned f_node_dict = find_dict( first_type ) ;
    unsigned l_node_dict = find_dict( last_type ) ; // given business -> col
    
    const dict< unsigned_Key , unsigned>* const dict_user_r = _r_idMapping[ f_node_dict ] ;

    // transform Business id to co
    vector<unsigned> given_business_col ;
    vector< pair< float , float> > given_business_pos ;
    transform_id_to_ind( given_business , given_business_col , given_business_pos , _idMapping[l_node_dict] ) ;

    // transform User id to row
    unsigned_Key* Key = new unsigned_Key( user ) ;
    unsigned Q_user = (*_idMapping[f_node_dict])[ *Key ]._ind ;
    delete Key ;

    sparse_matrix* spm_timeT = _offline_sim_matrix ;
    if( last_node-> _mask )
        spm_timeT = _offline_sim_matrix-> PairWise_dot( last_node-> _mask , true ) ;
    
    // calculate simularity score with threshold_dist
    vector<row_info> user_score ;
    if( threshold_dist == 0 ) {
        spm_timeT -> calculate_SIM( given_business_col , user_score ) ;
    }
    else {
        vector< pair< float , float> > user_pos ;
        user_pos.resize( spm_timeT-> getNumRows() ) ;
        getUserPosition( _idMapping[f_node_dict] , _r_idMapping[f_node_dict] , user_pos ) ;
        spm_timeT -> calculate_SIM( given_business_col , given_business_pos , user_pos , threshold_dist , user_score ) ;
    }

    const unsigned user_size = _offline_sim_matrix-> getNumRows() ;

    sort( user_score.begin() , user_score.end() , sort_by_row ) ;
    id_state* SS = new id_state[ user_size ] ; // index => row 
    unsigned candidate_num = user_size ;
    for( unsigned i = 0 , j = 0 ; i < user_size , j < user_score.size() ; i++ ) // initialize SS
    {
        if( user_score[j]._row == i ) {
            SS[i].score = user_score[j]._simScore ;
            j++ ;
        }
        else {
            SS[i].score = -1 ;
            SS[i].candidate = false ;
            candidate_num-- ;
        }
    }
    SS[ Q_user ].score = -1 ; // query user may not be in SS 
    SS[ Q_user ].candidate = false ;
    SS[ Q_user ].examined = true ;
    candidate_num-- ;

    sort( user_score.begin() , user_score.end() , sort_by_score ) ;
    vector<unsigned> L ;
    L.resize( user_score.size() ) ;
    for( unsigned i = 0 ; i < user_score.size() ; i++ ) { // the rest is don't care => score = 0
        L[i] = user_score[i]._row ; // L is SS in descending order , L[i] = SS's index 
    }
    ////////////////////////////////////////////////////////////////////////////////////
    const sparse_matrix* const UUrelation = find_relation_matrix( first_type , first_type ) ;
    fstream fout ;
    fout.open( outfile.c_str() , ios::out ) ;
    vector< unsigned>  N ; 
    Heap ret_U ;
    float* RS = new float[ user_size ] ;
    unsigned h = 0 ;
    float    r = 0 ;
    N.push_back( Q_user ) ;
    while( !N.empty() ) {
        vector<unsigned > H ;
        H.clear() ;
        h++ ;
        while( !N.empty() ) { // u
            unsigned this_id = N.back() ;
            N.pop_back() ;
            vector<unsigned> t_id_neighbor ;
            UUrelation-> get_neighbor( this_id , t_id_neighbor ) ;
            for( unsigned i = 0 ; i < t_id_neighbor.size() ; i++ )
            {
                unsigned temp = t_id_neighbor[i] ;
                if( SS[temp].examined == true ) continue ;
                H.push_back( temp ) ;
                SS[temp].examined = true ;
                if( SS[temp].candidate == false ) continue ;
                RS[temp] = ( float(SS[temp].score) / float(h) ) ;
                SS[temp].candidate = false ;
                candidate_num-- ;
                if( RS[temp] > r) {
                    heap_ele p( RS[temp] , temp ) ;
                    ret_U.insert( p ) ;
                    if( ret_U.size() > topK ) {
                        ret_U.pop_min() ;
                        r = ret_U.get_min() ;
                        unsigned prune_index = L.size() ;
                        for( unsigned j = 0 ; j < L.size() ; j++ ){
                            if( SS[ L[j] ].score < (r * h) ){
                                prune_index = j ;
                                break;
                            }
                        }
                        for( unsigned j = prune_index ; j < L.size() ; j++ ) {
                            if( SS[L[j]].candidate ) candidate_num--;
                            SS[ L[j] ].candidate = false ;
                        }
                        assert( candidate_num < user_size ) ; 
                        if( candidate_num == 0 ) {
                            vector<heap_ele> U = ret_U.getallData() ;
                            float total_score = 0 ;
                            for( unsigned k = 0 ; k < U.size() ; k++ ) {
                                unsigned_Key* Key = new unsigned_Key( U[k].second ) ;
                                float score = U[k].first;
                                total_score += score ;
                                fout << (*dict_user_r)[ *Key ] << '\t' << setprecision(5) << score ;
                                fout << setfill(' ') << setw(10) << setprecision(5) << total_score << '\n' ;
                            }
                            return true ;
                        }
                    }
                }
            }
        }
        N.clear() ; // N = H
        while( !H.empty()) {
            N.push_back( H.back()) ;
            H.pop_back() ;
        }
        
    }

    vector<heap_ele> U = ret_U.getallData() ;
    float total_score = 0 ;
    for( unsigned k = 0 ; k < U.size() ; k++ ) {
        unsigned_Key* Key = new unsigned_Key( U[k].second ) ;
        float score = U[k].first;
        total_score += score ;
        fout << (*dict_user_r)[ *Key ] << '\t' << setprecision(5) << score ;
        fout << setfill(' ') << setw(10) << setprecision(5) << total_score << '\n' ;
    }

}                               