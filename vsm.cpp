#include "vsm.hpp"
#include "model.hpp"
#include "common.hpp"

#include <vector>
#include <math.h>
#include <assert.h>
#include <tuple>

// (k + 1) tf / (k(1 - b + b * dl/avg_dl) + tf)
double OKapi::getTF(size_t doc_id, size_t grams_id){
  Document* doc = doc_container.docs_vector[doc_id];
  double tf = doc->getTf(grams_id);
  double dl = doc->doc_len;
  double avg_dl = doc_container.avg_doc_len;
  return ((OKapi_k + 1) * tf) / (OKapi_k * (1 - OKapi_b + OKapi_b * dl / avg_dl) + tf);
}

// for query
void OKapi::getTF(vector<string> & concepts_vec, vector<GramsInfo *>& term_vec){
  map<size_t, size_t> grams_id_to_tf_map;
  for(size_t i = 0 ; i < concepts_vec.size() ; ++i){
    string concept_str = concepts_vec[i];
    string gram;
    int first_id = -1, second_id = -1;
    for(size_t pos = 0 ; pos < concept_str.length() ; pos += 3){
      gram = concept_str.substr(pos, 3);
      if(gram == "Ｎ"){
        update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id("NBA"), -1);
        update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id("nba"), -1);
        break;
      }else if(gram == "Ｂ"){
        update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id("BOT"), -1);
        update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id("bot"), -1);
        break;
      }
      update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id(gram), -1);
      if((pos + 6) <= concept_str.length()){
        update_tf(grams_id_to_tf_map, voc_to_id_map.get_voc_id(gram), 
                  voc_to_id_map.get_voc_id(concept_str.substr(pos+3, 3)));
      }
    }
  }
  map<size_t, size_t>::iterator it = grams_id_to_tf_map.begin();
  while(it != grams_id_to_tf_map.end()){
    size_t grams_id = it->first;
    double tf = it->second;
    term_vec.push_back(new GramsInfo(grams_id, (OKapi_ka + 1) * tf / (OKapi_ka + tf)));
    ++it;
  }
}

void OKapi::update_tf(map<size_t, size_t>& grams_id_to_tf_map, int first_id, int second_id){
  if(first_id == -1) return;
  int grams_id = inverted_file.get_grams_id(make_pair(first_id, second_id));
  if(grams_id == -1) return;
  map<size_t, size_t>::iterator it = grams_id_to_tf_map.find(grams_id);
  if(it == grams_id_to_tf_map.end()){
    grams_id_to_tf_map.insert(make_pair(grams_id, 1));
  }else{
    ++it->second;
  }
}

// ln((N-df+0.5)/(df+0.5))
double IDF::getIDF(size_t grams_id){
  double N = doc_container.docs_vector.size();
  double df = inverted_file.getDF(grams_id);
  return log((N - df + 0.5) / (df + 0.5));
}

/*
  DocContainer & doc_container;
  InvertedFile & inverted_file;
  TFInterface tf;
  IDFInterface idf;
*/
void TFIDF::calculateScore(size_t doc_id, vector<double> & term_vector){
  for(size_t grams_id = 0 ; grams_id < term_vector.size() ; ++grams_id){
     term_vector[grams_id] = tf->getTF(doc_id, grams_id) * idf->getIDF(grams_id); 
  }    
}

// helper

TOPK_TERM_INFO_VEC * get_top_k_term(Document * doc, TFInterface * tf_interface, IDFInterface * idf, size_t k){
  TOPK_TERM_INFO_VEC * topk_term_tfidf = new TOPK_TERM_INFO_VEC;
  make_heap(topk_term_tfidf->begin(), topk_term_tfidf->end(), my_greater());
  unordered_map<size_t, size_t>::iterator tf_map_it = doc->tf_map.begin();
  while(tf_map_it != doc->tf_map.end()){
    size_t term_id = tf_map_it->first;
    size_t tf = tf_map_it->second;
    double tfidf = tf_interface->getTF(doc->id, term_id) * idf->getIDF(term_id);
    if(topk_term_tfidf->size() == k){
      TERMID_TF_TFIDF_TUPLE& tuple = topk_term_tfidf->front();
      if(get<2>(tuple) < tfidf){
        pop_heap(topk_term_tfidf->begin(), topk_term_tfidf->end(), my_greater()); topk_term_tfidf->pop_back();
        topk_term_tfidf->push_back(make_tuple(term_id, tf, tfidf)); push_heap(topk_term_tfidf->begin(), topk_term_tfidf->end(), my_greater());
      }
    }else if(topk_term_tfidf->size() < k){
      topk_term_tfidf->push_back(make_tuple(term_id, tf, tfidf)); push_heap(topk_term_tfidf->begin(), topk_term_tfidf->end(), my_greater());    
    }
    ++tf_map_it;
  }
  sort_heap(topk_term_tfidf->begin(), topk_term_tfidf->end(), my_greater());
  cout << "Finish doc " << doc->id << endl;
  return topk_term_tfidf;
}

TOPK_TERM_INFO_FOR_EACH_DOC_VEC * get_topk_term_for_each_doc(DocContainer & docContainer, TFInterface * tf, 
                                IDFInterface * idf, size_t k)
{
  TOPK_TERM_INFO_FOR_EACH_DOC_VEC* topk_term_for_each_doc = new TOPK_TERM_INFO_FOR_EACH_DOC_VEC;
  vector<Document *>& docs_vector = docContainer.docs_vector;
  for(size_t doc_id = 0 ; doc_id < docs_vector.size() ; ++doc_id){
    Document * doc = docs_vector[doc_id];
    topk_term_for_each_doc->push_back(get_top_k_term(doc, tf, idf, k));
  }
  return topk_term_for_each_doc;
}

void output_topk_inverted_file(InvertedFile& inverted_file, size_t topk, TOPK_TERM_INFO_FOR_EACH_DOC_VEC * topk_term_info_for_each_doc_vec_ptr){
  ofstream topk_inverted_file;
  std::ostringstream stringStream;
  stringStream << "topk_term_inverted_file/top_" << topk << "_inverted_file.csv";
  string topk_inverted_file_path = stringStream.str(); 
  topk_inverted_file.open(topk_inverted_file_path);
  if(!topk_inverted_file.is_open()){
    cout << "Cannot open topk inverted file" << endl;
    exit(1);
  }
  else{
    cout << "open " << topk_inverted_file_path << " successfully" << endl;
  }
  INVERTED_FILE_MAP inverted_file_map; 
  INVERTED_FILE_MAP::iterator it;
  TOPK_TERM_INFO_FOR_EACH_DOC_VEC& topk_term_info_for_each_doc_vec = *topk_term_info_for_each_doc_vec_ptr;
  for(size_t doc_id = 0 ; doc_id < topk_term_info_for_each_doc_vec.size() ; ++doc_id){
    TOPK_TERM_INFO_VEC& topk_term_info_vec = *topk_term_info_for_each_doc_vec[doc_id];
    for(size_t term_idx = 0 ; term_idx < topk_term_info_vec.size() ; ++term_idx){
      TERMID_TF_TFIDF_TUPLE& tuple = topk_term_info_vec[term_idx];
      size_t term_id = get<0>(tuple);
      size_t tf = get<1>(tuple);
      double tfidf = get<2>(tuple);
      it = inverted_file_map.find(term_id);
      if(it != inverted_file_map.end()){
        DOC_INFO_VEC * doc_info_vec = it->second;
        doc_info_vec->push_back(make_tuple(doc_id, tf, tfidf)); 
      }else{
        DOC_INFO_VEC* doc_info_vec = new DOC_INFO_VEC;
        doc_info_vec->push_back(make_tuple(doc_id, tf, tfidf));
        inverted_file_map.insert(make_pair(term_id, doc_info_vec));
      }
    }
  }
  it = inverted_file_map.begin();
  while(it != inverted_file_map.end()){
    size_t term_id = it->first;
    DOC_INFO_VEC& doc_info_vec = *(it->second);
    pair<int, int> grams = inverted_file.get_grams_by_id(term_id);
    topk_inverted_file << grams.first << " " << grams.second << " " << doc_info_vec.size() << "\n";
    for(size_t i = 0 ; i < doc_info_vec.size() ; ++i){
      DOC_ID_TF_TFIDF_TUPLE tuple = doc_info_vec[i];
      size_t doc_id = get<0>(tuple);
      size_t tf = get<1>(tuple);
      double tfidf = get<2>(tuple);
      topk_inverted_file << doc_id << " " << tf << " " << tfidf << "\n"; 
    }
    ++it;
  }
  cout << "Selected Term Number: " << inverted_file_map.size() << endl;
}
