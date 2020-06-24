#ifndef VSM_HPP
#define VSM_HPP

#include "model.hpp"
#include "common.hpp"

#include<map>

#ifndef OKapi_ka
#define OKapi_ka 175.0
#endif

#ifndef OKapi_k
#define OKapi_k 1.9
#endif

#ifndef OKapi_b
#define OKapi_b 0.75
#endif

#ifndef FEEDBACK_DOC_NUM
#define FEEDBACK_DOC_NUM 12
#endif

#ifndef FEEDBACK_TERM_NUM
#define FEEDBACK_TERM_NUM 70
#endif

#ifndef FEEDBACK_ALPHA
#define FEEDBACK_ALPHA 0.75
#endif

#ifndef FEEDBACK_BETA
#define FEEDBACK_BETA (1-FEEDBACK_ALPHA)
#endif

class VectorWeightAlgorithm{
  virtual void calculateScore(vector<double> &) = 0;
};

class TFInterface{
public:
  TFInterface(DocContainer & doc_container, InvertedFile & inverted_file, VocToIdMap& voc_to_id_map): 
  doc_container(doc_container), inverted_file(inverted_file), voc_to_id_map(voc_to_id_map){}

  virtual double getTF(size_t, size_t)= 0;
  virtual void getTF(vector<string> & concepts_vec, vector<GramsInfo*>& term_vec) = 0;
  DocContainer & doc_container;
  InvertedFile & inverted_file;
  VocToIdMap& voc_to_id_map;
};

class OKapi: public TFInterface{
public:
  OKapi(DocContainer & doc_container, InvertedFile & inverted_file, VocToIdMap& voc_to_id_map):
    TFInterface(doc_container, inverted_file, voc_to_id_map){} 
  double getTF(size_t, size_t);
  void getTF(vector<string> & concepts_vec, vector<GramsInfo*>& term_vec);
  void update_tf(map<size_t, size_t>& grams_id_to_tf_map, int first_id, int second_id);
};

class IDFInterface{
public:
  virtual double getIDF(size_t) = 0;
};

class IDF: public IDFInterface{
public:
  IDF(DocContainer & doc_container, InvertedFile & inverted_file): doc_container(doc_container), inverted_file(inverted_file){  }
  double getIDF(size_t);
  DocContainer & doc_container;
  InvertedFile & inverted_file;
};

class TFIDF: public VectorWeightAlgorithm {
public:
  TFIDF(DocContainer & doc_container, InvertedFile & inverted_file, TFInterface * tf, IDFInterface *idf): 
    doc_container(doc_container), inverted_file(inverted_file){
    this->tf = tf;
    this->idf = idf;
  }
  void calculateScore(size_t doc_id, vector<double> &);
  DocContainer & doc_container;
  InvertedFile & inverted_file;
  TFInterface * tf;
  IDFInterface * idf;
};

// helper

typedef tuple<size_t, size_t, double> DOC_ID_TF_TFIDF_TUPLE;
typedef vector<DOC_ID_TF_TFIDF_TUPLE> DOC_INFO_VEC;
typedef unordered_map<size_t, DOC_INFO_VEC *> INVERTED_FILE_MAP;

typedef tuple<size_t, size_t, double> TERMID_TF_TFIDF_TUPLE;
typedef vector<TERMID_TF_TFIDF_TUPLE> TOPK_TERM_INFO_VEC;
typedef vector<TOPK_TERM_INFO_VEC *> TOPK_TERM_INFO_FOR_EACH_DOC_VEC;

struct my_greater{
  bool operator()(const TERMID_TF_TFIDF_TUPLE& a,const TERMID_TF_TFIDF_TUPLE& b) const{
    return get<2>(a) > get<2>(b);
  }
};
TOPK_TERM_INFO_VEC * get_top_k_term(Document * doc, TFInterface * tf_interface, IDFInterface * idf, size_t k);
TOPK_TERM_INFO_FOR_EACH_DOC_VEC * get_topk_term_for_each_doc(DocContainer & docContainer, TFInterface * tf, 
                                IDFInterface * idf, size_t k);
void output_topk_inverted_file(InvertedFile& inverted_file, TOPK_TERM_INFO_FOR_EACH_DOC_VEC * topk_term_info_for_each_doc_vec_ptr);

#endif
