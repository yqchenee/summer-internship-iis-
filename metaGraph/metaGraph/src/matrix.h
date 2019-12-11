#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <vector>
#include <utility>
#include <climits>
#include "util.h"
using namespace std;

class row_info // BQT
{
public:
	row_info(const unsigned& r , const int& s ) :
		_row(r) , _simScore(s) {} 
	unsigned _row ;
	int      _simScore ;
	unsigned _socDist ;
};

class ele_info // get topK
{
public:
	ele_info() {}
	ele_info( const unsigned& r , const unsigned& c , const unsigned& w ) :
		_row(r) , _col(c) , _weight(w) {}
	ele_info( const unsigned& r , const unsigned& w ) :
		_row(r) , _weight(w) {}

	unsigned _row ;
	unsigned _col ;
	unsigned _weight ;
};

class sp_m_ele // element in sparse_matrix 
{
friend class sparse_matrix ;
public:
	sp_m_ele( const unsigned& c , const unsigned& ts , const unsigned& te , const int& w = 1 ) : \
		_col(c) , _timeStart(ts) , _timeEnd(te) , _weight(w) { }
	sp_m_ele( const sp_m_ele& ele ) { // make copy of ele
		_col = ele._col ;
		_timeStart = ele._timeStart ;
		_timeEnd = ele._timeEnd ;
		_weight = ele._weight ;
	}
	sp_m_ele( const unsigned& c , const int& w ) : _col(c) , _weight(w) {}
private:
	unsigned _col ;
	unsigned _timeStart ;
	unsigned _timeEnd ;
	int 	 _weight ;
};

class sparse_matrix // adjacency list
{
public:
	sparse_matrix() {}
	sparse_matrix( unsigned nR , unsigned nC ) : _numRows(0) , _numCols(0) , _matrix_rows(0) {
		init( nR , nC ) ;
	}
	sparse_matrix( const sparse_matrix& spm ) {
		_numRows = spm._numRows ;
		_numCols = spm._numCols ;
		_matrix_rows = spm._matrix_rows ;
	}
	~sparse_matrix() { reset() ; }

	void reset() { _numRows = 0 ; if(_matrix_rows) { delete[] _matrix_rows ; _matrix_rows = 0; } }
	void init( unsigned nR , unsigned nC ) {
		reset() ;
		_numRows 		= nR ;
		_numCols 		= nC ;
		_matrix_rows 	= new vector< sp_m_ele > [ _numRows ] ;
	}

	void update_ele( unsigned whichRow , const sp_m_ele& ele ) ;
	void insert_ele( unsigned whichRow , const sp_m_ele& ele ) ;
	void update_row( unsigned t_i , const vector<sp_m_ele >& ret_m_row_vector ) ;
	
	sparse_matrix* transposed() const ;
	sparse_matrix* dot( const sparse_matrix* const sp_m ) const ;
	sparse_matrix* PairWise_dot( const sparse_matrix* const sp_m , bool filter = false ) const ;
	void compare( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_b , vector<sp_m_ele>& ret ) const ;
	void filter( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_b , vector<sp_m_ele>& ret ) const ;

	sparse_matrix* get_matrix_at_time_T( const unsigned time ) const ;
	sparse_matrix* filter_dot( const sparse_matrix* const sp_m ) const ;
	sparse_matrix* filter_PairWise_dot( const sparse_matrix* const sp_m ) const ;
	void pw_newMultiply( const vector<sp_m_ele>& v_a , const vector<sp_m_ele>& v_b , vector<sp_m_ele>& ret ) const ;
	
	void get_top_K_ele( unsigned K , const string& outfile , \
					const dict< unsigned_Key , unsigned>* const  , \
					const dict< unsigned_Key , unsigned>* const ) ;
	void calculate_SIM( const vector<unsigned>& given_business , vector< row_info >& user_score ) ;
	void calculate_SIM( const vector<unsigned>& gb_col , const vector<pair<float,float>> &gb_pos , 
				const vector< pair<float , float>>& user_pos , float threshold_dist ,vector< row_info >& user_score ) ;
				
	void calculate_socDist( unsigned user , vector< row_info >& user_score ) ;

	void print_matrix() const ;
	unsigned size() const { 
		size_t size = 0 ;
		for( unsigned i = 0 ; i < _numRows ; i++ ) {
			for( unsigned j = 0 ; j < _matrix_rows[i].size() ; j++ ) {
				size += sizeof( _matrix_rows[i][j] ) ;
			}
		}
		return size ;
	}
	unsigned getNumCols() const { return _numCols ; } 
	unsigned getNumRows() const { return _numRows ; } 
	void get_neighbor( const unsigned& whichRow , vector<unsigned>& ret) const ;
private:
	unsigned                _numRows ;
	unsigned           		_numCols ;
	vector< sp_m_ele >*  	_matrix_rows ; // pair( col , weight ) 

};

class relation_matrix
{
public:
	relation_matrix( sparse_matrix* spm , const string& type_id1 , const string& type_id2 ) : \
					_matrix(spm) , _type_id1(type_id1) , _type_id2(type_id2) {}
	
	sparse_matrix* get_matrix() const {
		return _matrix ;
	}
	sparse_matrix* get_matrix_at_time_T( const unsigned& time ) {
		return _matrix-> get_matrix_at_time_T( time ) ;
	} 
	

	string get_type1() const {
		return _type_id1 ;
	}
	string get_type2() const {
		return _type_id2 ;
	}

private:
	sparse_matrix*			_matrix ;
	string					_type_id1 , _type_id2 ;
};

#endif // !MATRIX_H
