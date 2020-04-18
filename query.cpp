#include "query.hpp"
#include "vsm.hpp"
#include <assert.h>
#include <bits/stdc++.h>

using namespace std;
string Query::fromStringToId(string number){
  size_t len = number.length();
  return number.substr(len - 3, 3);
}

void Query::concepts_to_vector(string concepts){
  size_t pos = 0;
  string token;
  string delimeter = "、";
  vector<string> concepts_vec;
  while((pos = concepts.find(delimeter)) != string::npos){
    token = concepts.substr(0, pos);
    concepts_vec.push_back(token);
    concepts.erase(0, pos + delimeter.length());
  }
  delimeter = "。";
  if((pos = concepts.find(delimeter)) != string::npos){
    token = concepts.substr(0, pos);
    concepts_vec.push_back(token);
    concepts.erase(0, pos + delimeter.length());
  }

  tf->getTF(concepts_vec, this->term_vec);
}

struct greater1{
  bool operator()(const pair<size_t, double>& a,const pair<size_t, double>& b) const{
    return a.second > b.second;
  }
};
void Query::get_result(InvertedFile& inverted_file, DocContainer& doc_container, ofstream& rank_list_file, bool relevence_feedback){
  // get doc id through inverted_file
  set<size_t> list_id_vec;
  for(size_t i = 0 ; i < term_vec.size() ; ++i){
    size_t grams_id = term_vec[i]->grams_id;
    IndexInfo * index_info = inverted_file.inverted_index[grams_id];
    vector<FileInfo*>& file_list = index_info->file_list;
    for(size_t idx = 0 ; idx < file_list.size() ; ++idx){
      list_id_vec.insert(file_list[idx]->file_id);
    }  
  }

  // calculate the score
  vector<pair<size_t, double> > topk_doc_heap;
  make_heap(topk_doc_heap.begin(), topk_doc_heap.end(), greater1());
  set<size_t>::iterator it = list_id_vec.begin();
  while(it != list_id_vec.end()){
    double doc_score = 0.0;
    size_t doc_id = *it;
    for(size_t i = 0 ; i < term_vec.size() ; ++i){
      GramsInfo * grams_info = term_vec[i];
      double tf_score = tf->getTF(doc_id, grams_info->grams_id); 
      double idf_score = idf->getIDF(grams_info->grams_id); 
      double weight = grams_info->weight;
      doc_score += tf_score * idf_score * weight;
    }
    // push to topk Heap
    if(topk_doc_heap.size() == TOP_K){
      pair<size_t, double>& pair = topk_doc_heap.front();
      if(pair.second < doc_score){
        pop_heap(topk_doc_heap.begin(), topk_doc_heap.end(), greater1()); topk_doc_heap.pop_back();
        topk_doc_heap.push_back(make_pair(doc_id, doc_score)); push_heap(topk_doc_heap.begin(), topk_doc_heap.end(), greater1());
      }
    }else if(topk_doc_heap.size() < TOP_K){
      topk_doc_heap.push_back(make_pair(doc_id, doc_score)); push_heap(topk_doc_heap.begin(), topk_doc_heap.end(), greater1());    
    } 
    ++it;
  }

  // if relevence feadback 
  // Top10 doc, get term vector(tf * idf)
  // Top100 term
  // update query term vec
  // call get result again set feedback false
  // return 

  // print the topk ranked list
  sort_heap(topk_doc_heap.begin(), topk_doc_heap.end(), greater1());
  for(size_t i = 0 ; i < topk_doc_heap.size() ; ++i){
    pair<size_t, double> doc_id_and_score = topk_doc_heap[i];
    rank_list_file << doc_container.get_file_id_by_id(doc_id_and_score.first) << " ";
  }
  rank_list_file << endl;
}

void QueriesContainer::parse(string query_path, TFInterface * tf, IDFInterface * idf){
  ifstream query_file(query_path);
  if(query_file.is_open()){
    string line;
    // <?xml>
    getline(query_file, line);

    // <xml>
    getline(query_file, line);
    
    while(1){
    // <topic>
      getline(query_file, line);
      if(line == "</xml>") break;
      // <number><title><question><narrative><concept>
      string number = parseTag(query_file);
      string title = parseTag(query_file);
      string question = parseTag(query_file);
      string narrative = parseTag(query_file);
      string concepts = parseTag(query_file);
      
      // </topic>
      getline(query_file, line);
      query_list.push_back(new Query(number, title, question, narrative, concepts, tf, idf));
    }
  }else{
    cout << "unable to open file" << endl;
    exit(1);
  }
}
string QueriesContainer::parseTag(ifstream& xml_file){
  string tag;
  string content;
  char newline;
  getline(xml_file, tag, '>');
  if(xml_file.peek() == '\n') xml_file.get(newline);
  getline(xml_file, content, '<');
  if(content.back() == '\n'){
    content.pop_back();
  }
  getline(xml_file, tag, '>');
  if(xml_file.peek() == '\n') xml_file.get(newline);
  return content;
}

void QueriesContainer::get_result(InvertedFile& inverted_file, DocContainer& doc_container, string rank_list_path, bool relevence_feedback){
  ofstream rank_list_file;
  rank_list_file.open(rank_list_path);
  if(!rank_list_file.is_open()){
    cout << "Cannot open rank list file" << endl;
    exit(1);
  }
  rank_list_file << "query_id,retrieved_docs" << endl;
  for(size_t i = 0 ; i < query_list.size() ; ++i){
    rank_list_file << query_list[i]->id << ",";
    query_list[i]->get_result(inverted_file, doc_container, rank_list_file, relevence_feedback);
  }
}
// helper function



