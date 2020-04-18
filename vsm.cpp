#include "vsm.hpp"
#include "model.hpp"
#include "common.hpp"

#include <vector>
#include <math.h>
#include <assert.h>

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
