#ifndef META_GRAPH_H
#define META_GRAPH_H

#include "util.h"
#include "matrix.h"
#include <iostream>
#include <cassert>

using namespace std;

class node
{
friend class metaGraph ;
public:
    node() {}
	node( const string& type , const unsigned& id , const unsigned& level) 
            : _id(id) , _level(level) , _type(type)
            {
                _childs.clear() ;
                _parents.clear() ;
                _rec_matrix = 0 ;
                _mask = 0 ;
                _last_split = 0 ;
            }
    void push_child( node* n ) { _childs.push_back(n); }
    void push_parent( node* n ) { _parents.push_back(n); }
    void reset_mask() { if(_mask) delete _mask ; _mask = 0 ;}
private:
 	unsigned 			_id , _level ; // node assignment
	string 				_type ;        // node assignment
    node*               _last_split ;
    sparse_matrix*      _rec_matrix ; // simularity , in main_counting
    sparse_matrix*      _mask ;     // filter , in count_mask
	vector<node*> 	    _childs ; // path assignment
	vector<node*> 	    _parents ; // path assignment
};

class id_info
{
public:
    id_info( const unsigned& id , const float& lo , const float& la ) 
            : _ind( id ) , _longtitude( lo ) , _latitude( la ) {}
    unsigned _ind ;
    float    _longtitude ;
    float    _latitude ;
};

class metaGraph
{
public:
    metaGraph() { 
        _level_node_index_in_nodeL = 0 ;
        _offline_sim_matrix = 0 ;
    } 
    // read input file
    bool read_info( const string& infile  ) ;
    void readGraph_id( const string& infile , const string& type_id ) ;
    void readGraph_rel( const string& file_path , const string& type_a , const string& type_b ) ;
    bool read_topology( ifstream& fin ) ;

    // read query file
    bool readQueryFile( const string& qF , unsigned& topK , string& outfile ) ;
    bool readQueryFile( const string& qF , int& Time , unsigned& topK , unsigned& user , 
                        string& businessFile , string& outfile , float& threshold_dist  ) ;
    bool readQueryFile( const string& qF , int& Time , unsigned& user , string& businessFile , 
                            string& k_user_File , string& outfile , float& threshold_dist ) ;
    //bool readQueryFile( const string& qF , string& off_sim , int& Time , unsigned& topK , unsigned& user , 
      //                  string& businessFile , string& outfile , float& threshold_dist  ) ;

    // read offline simularity matrix 
    bool read_offline_simularity( string offline_Sim ) ;

    // matrix operation
    void main_counting() ;
    void count_mask( const unsigned time = 0 ) ;
    void reset_mask() {
        for( unsigned i = 0 ; i < _nodeList.size() ; i++ ) {
            _nodeList[i]-> reset_mask() ;
        }
    }
    unsigned find_dict( const string& type_id ) {
        for( unsigned i = 0 ; i < _index_to_type_id.size() ; i++ ) {
            if( _index_to_type_id[i].compare( type_id ) == 0 ) return i ;
        }
        cout << "Error: in Relation matrix, cannot find type_id: " << type_id << endl;
        assert(0) ; 
    }
    sparse_matrix* find_relation_matrix( const string& type1 , const string& type2 ) ;

    // first program
    void output_final_matrix( const string& outfile , const unsigned topK ) ;
    // second program
    void output_result_with_BQT( const string& outfile , const vector<unsigned>& given_business , 
                                unsigned user , unsigned topK , float threshold_dist ) ;
    void output_topK( const string& outfile , vector<row_info>& user_score ,\
                    unsigned topK , const dict< unsigned_Key , unsigned>* const idM ) ;
    // third program
    bool calculate_final_query_score( const string& outfile , const vector<unsigned>& given_business , const vector<unsigned>& k_user , 
                                        unsigned user , float threshold_dist ) ;
    bool calculate_query_k_user( const string& outfile , const vector<row_info>& user_score , const vector<unsigned>& k_user_row ) ;
    // forth program
    bool online_Query_Prunning( const string& outfile , const vector<unsigned>& given_business , 
                                unsigned user , unsigned topK , float threshold_dist ) ;

    // get all user position
    void getUserPosition( const dict< unsigned_Key , id_info>* const _idM ,
                    const dict< unsigned_Key , unsigned>* const r_idM , vector<pair <float , float>>& user_pos ) ;
private:
    unsigned        _num_ids , _num_rel , _num_level ; 
    
    // readin idFile and mapping id to index
    vector< dict< unsigned_Key , id_info>* >    _idMapping ;
    vector< dict< unsigned_Key , unsigned>* >   _r_idMapping ;
    vector< string >                            _index_to_type_id ;   // [id_mapping_index] = type id ?  
    
    // readin relationFile and construct rel_matrix
    vector< relation_matrix >                 	_rel_matrix ;
    sparse_matrix*                              _offline_sim_matrix ;

    // read in metagraph topology
    // _level_node_index_in_nodeL record the nodes(xx) in defferent level of metagraph
    // use _nodeList[ xx ] to get this node
    vector< node* >                             _nodeList ;
    vector< unsigned >*                         _level_node_index_in_nodeL ;
};



#endif // !METAGRAPH_H
