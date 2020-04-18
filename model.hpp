#ifndef MODEL_HPP
#define MODEL_HPP

#include<iostream>
#include<fstream>
#include<unordered_map>
#include<vector>
#include<sys/stat.h>
#include<boost/functional/hash.hpp>
#include<boost/algorithm/string.hpp>

using namespace std;

class FileInfo{
  public: 
  FileInfo(size_t file_id, size_t tf){
    this->file_id = file_id;
    this->tf = tf;
  }

  void print(){
    cout << "File Id: " << file_id << " " << "Term freq: " << tf << endl;
  }

  size_t file_id;
  size_t tf;
};

class IndexInfo{
  public:
  IndexInfo(int first_term, int second_term, size_t file_num){
    this->grams = make_pair(first_term, second_term);
    this->df = file_num;
  }

  void add(size_t file_id, size_t ft){
    file_list.push_back(new FileInfo(file_id, ft));
  }

  void print(){
    cout << "Document Frequency: " << df << endl;
    for(size_t i = 0 ; i < file_list.size() ; ++i){
      file_list[i]->print();
    }
  }
  vector<FileInfo *> file_list;
  pair<int, int> grams;
  size_t df;
};

class InvertedFile{
public:
  InvertedFile(string inverted_file_path){
    ifstream inverted_file(inverted_file_path);
    size_t id = 0;
    if(inverted_file.is_open()){
      while(!inverted_file.eof()){
        int first_term, second_term ,file_num;
        size_t file_id, tf;
        inverted_file >> first_term >> second_term >> file_num;
        IndexInfo* index_info = new IndexInfo(first_term, second_term, file_num);
        for(size_t i = 0 ; i < file_num ; ++i){
          inverted_file >> file_id >> tf;
          index_info->add(file_id, tf);
        }
        inverted_index.push_back(index_info);
        grams_to_id.insert(make_pair(make_pair(first_term, second_term), id++));
      }
    }else{
      cout << "unable to open file" << endl;
      exit(1);
    }
    inverted_file.close();
  }

  int get_grams_id(pair<int, int> grams){
    unordered_map<pair<int, int>, size_t, boost::hash<pair<int, int> > >::iterator it = grams_to_id.find(grams);
    if(it != grams_to_id.end()){
      return it->second;
    }
    return -1;
  }

  pair<int, int>& get_grams_by_id(size_t id){
    IndexInfo * index_info = inverted_index[id];
    return index_info->grams;
  }

  size_t getDF(size_t grams_id){ 
    IndexInfo * index_info = inverted_index[grams_id];
    return index_info->df;
  }

  vector<IndexInfo *> inverted_index;
  unordered_map<pair<int, int>, size_t, boost::hash<pair<int, int> > > grams_to_id;
};

class Document{
public:
  Document(size_t id, string filePath){
    this->id = id;
    this->doc_len = get_file_size(filePath);
    this->file_path = filePath;
  }

  size_t get_file_size(string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
  }

  void set_tf(size_t grams_id, size_t tf){
    tf_map.insert(make_pair(grams_id, tf));
  }
    
  size_t getTf(size_t grams_id){
    unordered_map<size_t, size_t>::iterator it = tf_map.find(grams_id);
    if(it == tf_map.end()) return 0;
    return it->second;
  }
  void print_tf(){
    unordered_map<size_t, size_t>::iterator it = tf_map.begin();
    while(it != tf_map.end()){
      cout << "(" << it->first << ") " << it->second << endl;
      ++it;
    }
  }

  string get_file_id(){
    string file_path = this->file_path;
    string token;
    string delimeter = "/";
    size_t pos;
    while((pos = file_path.find(delimeter)) != string::npos){
      file_path.erase(0, pos + delimeter.length());
    }
    return boost::algorithm::to_lower_copy(file_path);
  }
  
  size_t id;
  size_t doc_len;
  string file_path;
  vector<int> term_vector;
  // (grams_id, tf)
  unordered_map<size_t, size_t> tf_map;
};

class DocContainer{
public:
  DocContainer(string file_list_path, string NCTIR_path){
    ifstream file_list(file_list_path);
    if(file_list.is_open()){
      string line;
      size_t id = 0;
      while(getline(file_list, line)){
        string file_path = NCTIR_path + line;
        docs_vector.push_back(new Document(id++, file_path));
      }
    }else{
      cout << "unable to open file" << endl;
      exit(1);
    }
    file_list.close();
    avg_doc_len = 0.0;
    for(size_t i = 0 ; i < docs_vector.size() ; ++i){
      avg_doc_len += docs_vector[i]->doc_len;
    }
    avg_doc_len /= (double)docs_vector.size();
  }

  void set_docs_tf(InvertedFile & inverted_file){
    vector<IndexInfo *>& inverted_index = inverted_file.inverted_index;
    for(size_t grams_id = 0 ; grams_id < inverted_index.size() ; ++grams_id){
      vector<FileInfo *>& file_list = inverted_index[grams_id]->file_list;
      for(size_t i = 0 ; i < file_list.size() ; ++i){
        FileInfo * file_info = file_list[i];
        Document * doc = docs_vector[file_info->file_id];
        doc->set_tf(grams_id, file_info->tf);
      }
    }
  }
  
  string get_file_id_by_id(size_t id){
    return docs_vector[id]->get_file_id();
  } 
  vector<Document *> docs_vector;
  double avg_doc_len;
};

class VocToIdMap{
public:
  VocToIdMap(string voc_list_path){
    ifstream voc_list(voc_list_path);
    if(voc_list.is_open()){
      string line;
      size_t id = 1;
      // utf8 
      getline(voc_list, line);
      while(getline(voc_list, line)){
        voc_to_id.insert(make_pair(line, id++));
      }
    }else{
      cout << "unable to open file" << endl;
      exit(1);
    }
    voc_list.close();
  }

  int get_voc_id(string voc){
    unordered_map<string, size_t>::iterator it = voc_to_id.find(voc);
    if(it != voc_to_id.end()) return it->second;
    return -1;
  }

  unordered_map<string, size_t> voc_to_id;
};


#endif
