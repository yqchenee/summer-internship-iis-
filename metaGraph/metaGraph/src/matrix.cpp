#include "matrix.h"
#include <cassert>
#include <climits>
#include <fstream> // outputFile
#include <algorithm>
#include <iomanip>

using namespace std;

// update weight if ele is already in matrix
// else insert it
void sparse_matrix::update_ele( unsigned whichRow , const sp_m_ele& ele ) 
{
    // cout << whichRow << ' ' << whichCol << ' ' << weight << endl;
    for( unsigned j = 0 ; j < _matrix_rows[ whichRow ].size() ; j++ ) { // which_col
        if( _matrix_rows[whichRow][j]._col == ele._col ) {
            _matrix_rows[whichRow][j]._weight += ele._weight ;
            return ;
        }
    }
    insert_ele( whichRow , ele ) ;
}

// not in the matrix
void sparse_matrix::insert_ele( unsigned whichRow , const sp_m_ele& ele ) // weight = 1
{
    _matrix_rows[ whichRow ].push_back( ele ) ; 
}

void sparse_matrix::update_row( unsigned whichRow , const vector<sp_m_ele>& B_m_row_vector ) 
{
    vector<sp_m_ele>& this_m_row_vector =  _matrix_rows[whichRow] ;
    if( this_m_row_vector.size() > 50 ) {
        dict< unsigned_Key , unsigned >* this_m_col = new dict< unsigned_Key , unsigned > ( this_m_row_vector.size() ) ;

        for( unsigned i = 0 ; i < this_m_row_vector.size() ; i++ ) {
            const sp_m_ele& ele = this_m_row_vector[i] ; 
            if( ele._weight ) {
                unsigned_Key* Key = new unsigned_Key( ele._col ) ; 
                this_m_col-> write( *Key , i ) ;
                delete Key ;
            }
        }

        for( unsigned j = 0 ; j < B_m_row_vector.size() ; j++ )
        {
            const sp_m_ele& ele = B_m_row_vector[j] ;
            unsigned_Key* Key = new unsigned_Key( ele._col ) ;
            unsigned col ;
            if( this_m_col-> read( *Key , col ) ) {
                this_m_row_vector[ col ]._weight += ele._weight ;
            }
            else insert_ele( whichRow , ele ) ;
            delete Key ;
        }
        delete this_m_col ;
        return ;
    }
    // vb is small
    for( unsigned i = 0 ; i < B_m_row_vector.size() ; i++ )
    {
        update_ele( whichRow , B_m_row_vector[i] ) ;
    }
}

// no matter what the time
sparse_matrix* sparse_matrix::dot( const sparse_matrix* const matrix_B ) const // this * matrix_B
{
    assert( _numCols == matrix_B-> _numRows ) ;
    //cout << "dot (" << _numRows << ',' << _numCols << ") (" << matrix_B->_numRows << ',' << matrix_B->_numCols << ")" << endl; 

    sparse_matrix* ret_m = new sparse_matrix( _numRows , matrix_B-> _numCols ) ;
    const vector< sp_m_ele >* const MATRIX_B = matrix_B-> _matrix_rows ;

    for( unsigned t_i = 0 ; t_i < _numRows ; t_i++ ) 
    {
        //cout << t_i << ' ' << _matrix_rows[t_i].size() << '\n' ;

        for( unsigned t_j = 0 ; t_j < _matrix_rows[ t_i ].size() ; t_j++ )
        {
            // ( i , t_ij_ele._col ) ele of matrix
            // ( t_col , ts , te , w )
            const sp_m_ele& t_ij_ele = _matrix_rows[ t_i ][ t_j ] ; 
            vector< sp_m_ele > ret_m_row_vector ;

            // multiplied by matrix_B's (t_ij_ele._col)'s row
            for( unsigned B_j = 0 ; B_j < MATRIX_B[ t_ij_ele._col ].size() ; B_j++ ) // t_col row of _matrix_B
            {
                int rec = ( t_ij_ele._weight * MATRIX_B[t_ij_ele._col][B_j]._weight ) ; // (t_i , p.f) * (p.f, matrix_B[p.f][spmj].f)
                sp_m_ele* ele = new sp_m_ele( MATRIX_B[t_ij_ele._col][B_j]._col , rec ) ;
                //ret_m-> update_ele( t_i , *ele ) ;
                ret_m_row_vector.push_back( *ele ) ;
            }
            ret_m -> update_row( t_i , ret_m_row_vector ) ;
        }
    }
    return ret_m ;
}
// matter the time
sparse_matrix* sparse_matrix::transposed() const
{
    //cout << "transposed "<< endl;
    sparse_matrix* ret_m  = new sparse_matrix( _numCols , _numRows ) ;
    for( unsigned i = 0 ; i < _numRows ; i++ ) 
    {
        for( unsigned j = 0 ; j < _matrix_rows[ i ].size() ; j++ )
        {
            const sp_m_ele& ij_ele = _matrix_rows[i][j] ;
            sp_m_ele* ele = new sp_m_ele( i , ij_ele._timeStart , ij_ele._timeEnd , ij_ele._weight ) ;
            ret_m-> update_ele( ij_ele._col , *ele ) ;
        }
    }
    return ret_m ;
}   

// no matter the time
// if filter => simularity = simularity + filter 
sparse_matrix* sparse_matrix::PairWise_dot( const sparse_matrix* const matrix_B , bool filter ) const 
{
    assert( _numRows == matrix_B-> _numRows ) ;
    assert( _numCols == matrix_B-> _numCols ) ;
    //cout << "pairwise dot " << filter << endl;

    sparse_matrix* ret_m = new sparse_matrix( _numRows , _numCols ) ;
    const vector< sp_m_ele >* const B_matrix_rows = matrix_B-> _matrix_rows ;

    if( !filter ) {
        for( unsigned i = 0 ; i < _numRows ; i++ )
        {
            this-> compare( _matrix_rows[i] , B_matrix_rows[i] , ret_m-> _matrix_rows[i] ) ;
        }
    }
    else { // in simularity * filter 
        for( unsigned i = 0 ; i < _numRows ; i++ )
        {
            this-> filter( _matrix_rows[i] , B_matrix_rows[i] , ret_m-> _matrix_rows[i] ) ;
        }
    }
    return ret_m ;
}

// ret = simularity + filter 
void sparse_matrix::filter( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_filter , vector<sp_m_ele>& ret ) const 
{
    int* filter_col = new int[ _numCols ] ;
    for( unsigned i = 0 ; i < _numCols ; i++ ) filter_col[i] = 0 ;
    for( unsigned j = 0 ; j < v_filter.size() ; j++ ) {
        filter_col[ v_filter[j]._col ] = v_filter[j]._weight ;
    }

    for( unsigned i = 0 ; i < v_a.size() ; i++ )
    {
        unsigned which_col = v_a[i]._col ;
        sp_m_ele* ele = new sp_m_ele( which_col , v_a[i]._weight + filter_col[ which_col ] ) ;
        ret.push_back( *ele ) ;
    }
}

void sparse_matrix::compare( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_b , vector<sp_m_ele>& ret ) const 
{
    for( unsigned i = 0 ; i < v_a.size() ; i++ )
    {
        unsigned which_col = v_a[i]._col ;
        for( unsigned j = 0 ; j < v_b.size() ; j++ ) {
            if( which_col == v_b[j]._col ) { // only appear one time
                sp_m_ele* ele = new sp_m_ele( which_col , (v_a[i]._weight * v_b[j]._weight ) ) ;
                ret.push_back( *ele ) ;
                break ;
            }
        }
    }
}

// filter the time
// offline & t => 0 
// offline & ~t => -x
sparse_matrix* sparse_matrix::get_matrix_at_time_T( const unsigned time ) const 
{
    sparse_matrix* ret_m = new sparse_matrix( _numRows , _numCols ) ;
    for( unsigned i = 0 ; i < _numRows ; i++ )
    {
        for( unsigned j = 0 ; j < _matrix_rows[i].size() ; j++ ) 
        {
            const sp_m_ele& ij_ele = _matrix_rows[i][j] ;
            if( ij_ele._timeStart > time || ij_ele._timeEnd < time ) {
                sp_m_ele* ele = new sp_m_ele( ij_ele._col , ( -ij_ele._weight ) ) ;
                ret_m-> insert_ele( i , *ele ) ; 
            }
            else {
                sp_m_ele* ele = new sp_m_ele( ij_ele._col , 1 ) ;
                ret_m-> insert_ele( i , *ele ) ;
            }
        }
    }
    return ret_m ;
}

inline int new_multiply( const int& a , const int& b ) {
    if( a == 1 && b == 1 ) return 0 ;
    else if( a == 1 ) return b ;
    else if( b == 1 ) return a ;
    else return -(a*b) ;
}
// in filter dot filter , apply new multiplication
sparse_matrix* sparse_matrix::filter_dot( const sparse_matrix* const matrix_B ) const // this * matrix_B
{
    assert( _numCols == matrix_B-> _numRows ) ;
    //cout << "filter dot (" << _numRows << ',' << _numCols << ") (" << matrix_B->_numRows << ',' << matrix_B->_numCols << ")" << endl; 

    sparse_matrix* ret_m = new sparse_matrix( _numRows , matrix_B-> _numCols ) ;
    const vector< sp_m_ele >* const MATRIX_B = matrix_B-> _matrix_rows ;

    for( unsigned t_i = 0 ; t_i < _numRows ; t_i++ ) 
    {
        for( unsigned t_j = 0 ; t_j < _matrix_rows[ t_i ].size() ; t_j++ )
        {
            const sp_m_ele& t_ij_ele = _matrix_rows[ t_i ][ t_j ] ; 
            vector< sp_m_ele > ret_m_row_vector ;
            for( unsigned B_j = 0 ; B_j < MATRIX_B[ t_ij_ele._col ].size() ; B_j++ ) 
            {
                int a = t_ij_ele._weight ;
                int b = MATRIX_B[ t_ij_ele._col ][ B_j ]._weight ;
                int rec = new_multiply( a , b ) ;
                if( rec == 0 ) continue ;
                sp_m_ele* ele = new sp_m_ele( MATRIX_B[t_ij_ele._col][B_j]._col , rec ) ;
                ret_m_row_vector.push_back( *ele ) ;
            }
            ret_m -> update_row( t_i , ret_m_row_vector ) ;
        }
    }
    return ret_m ;
}
// apply new multiplication , in filter* filter
sparse_matrix* sparse_matrix::filter_PairWise_dot( const sparse_matrix* const matrix_B ) const 
{
    assert( _numRows == matrix_B-> _numRows ) ;
    assert( _numCols == matrix_B-> _numCols ) ;
    //cout << "filter pairwise dot " << endl;

    sparse_matrix* ret_m = new sparse_matrix( _numRows , _numCols ) ;
    const vector< sp_m_ele >* const B_matrix_rows = matrix_B-> _matrix_rows ;

    for( unsigned i = 0 ; i < _numRows ; i++ )
    {
       pw_newMultiply( _matrix_rows[i] , B_matrix_rows[i] , ret_m-> _matrix_rows[i] ) ;
    }
    return ret_m ;
}

void sparse_matrix::pw_newMultiply( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_b , vector<sp_m_ele>& ret ) const 
{
    for( unsigned i = 0 ; i < v_a.size() ; i++ )
    {
        unsigned which_col = v_a[i]._col ;
        for( unsigned j = 0 ; j < v_b.size() ; j++ ) {
            if( which_col == v_b[j]._col ) { // only appear one time
                int rec = new_multiply(v_a[i]._weight , v_b[j]._weight ) ;
                if( rec == 0 ) break ;

                sp_m_ele* ele = new sp_m_ele( which_col , rec ) ;
                ret.push_back( *ele ) ;
                break ;
            }
        }
    }
}


bool func( const ele_info& i , const ele_info& j ) 
{
    return( i._weight > j._weight ) ;
}
void sparse_matrix::get_top_K_ele( unsigned topk , const string& outfile , \
					const dict< unsigned_Key , unsigned>* const first_dict , \
					const dict< unsigned_Key , unsigned>* const last_dict ) 
{
    fstream fout ;
    cout << "====================================================" << endl;
    if( topk == 0 ) {
        string out = "offline/" ;
        out = out + outfile ;
        fout.open( out.c_str() , fstream::out ) ;
        cout << "output offline matrix to : " << out << endl;
        topk = _numCols ;
        for( unsigned i = 0 ; i < _numRows ; i++ )
        {
            const vector<sp_m_ele>& row = _matrix_rows[i] ;
            unsigned max = ( topk < row.size() ) ? topk : row.size() ;
            for( unsigned n = 0 ; n < max ; n++ )
            {
                const sp_m_ele& ele = row[n] ;
                
                unsigned_Key* f_K = new unsigned_Key( i ) ;
                unsigned_Key* l_K = new unsigned_Key( ele._col ) ;
                fout << first_dict-> get(*f_K) ;
                fout << setfill(' ') << setw(10) <<  last_dict-> get(*l_K) ;
                fout << setfill(' ') << setw(10) <<  ele._weight << '\n' ;
            }
            
        }
    }
    else {
        fout.open( outfile.c_str() , fstream::out ) ;
        cout << "get top ( " << topk << " ) elements." << endl;

        for( unsigned i = 0 ; i < _numRows ; i++ )
        {
            //cout << i << '\n' ;
            vector< ele_info > row_ele ; // memorize row's element
            const vector< sp_m_ele >& row = _matrix_rows[i] ;
            for( unsigned j = 0 ; j < row.size() ; j++ )
            {
                ele_info* ele = new ele_info( i , row[j]._col , row[j]._weight ) ;
                row_ele.push_back( *ele ) ;
            }
            sort( row_ele.begin() , row_ele.end() , func ) ;
            
            unsigned max = ( topk < row_ele.size() ) ? topk : row_ele.size() ;
            for( unsigned n = 0 ; n < max ; n++ )
            {
                const ele_info& ele = row_ele[n] ;
                if( ele._weight <= 0 ) break;
                unsigned_Key* f_K = new unsigned_Key( ele._row ) ;
                unsigned_Key* l_K = new unsigned_Key( ele._col ) ;
                fout << first_dict-> get(*f_K) ;
                fout << setfill(' ') << setw(10) <<  last_dict-> get(*l_K) ;
                fout << setfill(' ') << setw(10) <<  ele._weight << '\n' ;
            }
            
        }
    }
}

// for given business , for each user count it total simularity score to given business
// if total score > 0 => push_back in user_score
void sparse_matrix::calculate_SIM( const vector<unsigned>& given_business_col , vector<row_info>& user_score ) 
{
    bool* gb_col = new bool[ _numCols ] ;
    for( unsigned i = 0 ; i < _numCols ; i++ ) {
        gb_col[i] = false ;
    }
    for( unsigned i = 0 ; i < given_business_col.size() ; i++ ) {
        gb_col[ given_business_col[i] ] = true ;
    }

    for( unsigned i = 0 ; i < _numRows ; i++ )
    {
        const vector< sp_m_ele >& row = _matrix_rows[i] ;
        unsigned score = 0 ;
        for( unsigned j = 0 ; j < row.size() ; j++ )
        {
            if( gb_col[ row[j]._col ] ) {
                score += row[j]._weight ;
            }
        }
        if( score > 0 ) {
            row_info* r = new row_info( i , score ) ;
            user_score.push_back( *r ) ;
        }
    } 
}

// for each user , first test it distance to all business  
// and count it total simularity score to all given business
// if total score > 0 => push_back in user_score
void sparse_matrix::calculate_SIM( const vector<unsigned>& given_b_col , const vector<pair<float,float>>& given_b_pos , 
				const vector< pair<float , float>>& user_pos , float threshold_dist ,vector< row_info >& user_score )
{
    cout << "query with threshold : " << threshold_dist << endl;
    typedef pair<float , float> pos_t ;

    unsigned gb_size = given_b_col.size() ;
    int* gb_col = new int[ _numCols ] ;
    for( unsigned i = 0 ; i < _numCols ; i++ ) {
        gb_col[i] = -1 ;
    }
    for( unsigned i = 0 ; i < gb_size ; i++ ) {
        gb_col[ given_b_col[i] ] = i ;
    }

    for( unsigned i = 0 ; i < _numRows ; i++ )
    {
        bool valid_user = true ; // user to all given_business distance must lower than threshold dist
        for( unsigned j = 0 ; j < gb_size ; j++ ) {
            pos_t b_pos = given_b_pos[ j ] ;
            pos_t u_pos = user_pos[ i ] ;
            if( countRealDistance( b_pos.first , b_pos.second , u_pos.first , u_pos.second ) > threshold_dist ) {
                valid_user = false ;
                break ;
            }
        }
        if( !valid_user ) continue ;

        const vector< sp_m_ele >& row = _matrix_rows[i] ;
        unsigned score = 0 ;
        for( unsigned j = 0 ; j < row.size() ; j++ )
        {
            int gb_pos_ind = gb_col[ row[j]._col ] ;
            if( gb_pos_ind == -1 ) // col not in given business 
                continue ;
            else  
                score += row[j]._weight ;
        }
        if( score > 0 ) {
            row_info* r = new row_info( i , score ) ;
            user_score.push_back( *r ) ;
        }
    }
}

#include <queue> // BFS search each user SOC_DIST to query_userQ (= user_row )
void sparse_matrix::calculate_socDist( unsigned user_row , vector<row_info>& user_score ) 
{
    assert( _numRows == _numCols ) ;
    unsigned* dist2Q = new unsigned[ _numRows ] ;
    for( unsigned i = 0 ; i < _numRows ; i++ ) {
        dist2Q[i] = UINT_MAX ; // dist
    }

    queue< pair< unsigned , unsigned >> frontier ; // row_ind , dist
    unsigned dist = 0 ;
    pair< unsigned , unsigned > p( user_row , dist ) ;
    frontier.push( p ) ;
    dist2Q[ user_row ] = 0 ;
    
    while( !frontier.empty() ) {
        pair< unsigned , unsigned > ff = frontier.front() ;
        frontier.pop() ;
        const vector<sp_m_ele>& mat_row = _matrix_rows[ ff.first ] ;
        dist = ff.second ;
        for( unsigned i = 0 ; i < mat_row.size() ; i++ ) {
            if( dist2Q[ mat_row[i]._col ] == UINT_MAX ) {
                pair< unsigned , unsigned > ip( mat_row[i]._col , dist+1 ) ;
                frontier.push( ip ) ;
                dist2Q[ mat_row[i]._col ] = (dist+1) ;
            }
        } 
    }

    for( unsigned i = 0 ; i < user_score.size() ; i++ ) {
        user_score[i]._socDist = dist2Q[ user_score[i]._row ] ;
    }
}

void sparse_matrix::get_neighbor( const unsigned& whichRow , vector<unsigned>& ret) const
{
    ret.clear() ;
    const vector<sp_m_ele>& row = _matrix_rows[whichRow] ;
    for( unsigned i = 0 ; i < row.size() ; i++ ) {
        ret.push_back( row[i]._col ) ;
    }
}

void sparse_matrix::print_matrix() const 
{
    int** whole_matrix = new int* [_numRows] ;
    for( unsigned i = 0 ; i < _numRows ; i++ ) 
    {
        whole_matrix[i] = new int [_numCols] ;
        for( unsigned j = 0 ; j < _numCols ; j++ ) 
        {
            whole_matrix[i][j] = 0 ;
        }
    }
    
    for( unsigned i = 0 ; i < _numRows ; i++ ) {
        for( unsigned j = 0 ; j < _matrix_rows[i].size() ; j++ ) {
            const sp_m_ele& p = _matrix_rows[i][j] ;
            whole_matrix[i][ p._col ] = p._weight ;
        }
    }

    for( unsigned i = 0 ; i < _numRows ; i++ ) {
        for( unsigned j = 0 ; j < _numCols ; j++ ) 
        {
            cout << whole_matrix[i][j] << ' ' ;
        }
        cout << '\n' ;
    }

    for( unsigned i = 0 ; i < _numRows ; i++ ) 
    {
        delete [] whole_matrix[i] ;
        whole_matrix[i] = 0 ;
    }
    delete[] whole_matrix ; 
    whole_matrix = 0 ;
}