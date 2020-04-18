#include<iostream>
#include<fstream>
#include<unordered_map>
#include<vector>
#include<sys/stat.h>
#include<boost/functional/hash.hpp>
#include "query.hpp"
#include "model.hpp"
#include "vsm.hpp"

using namespace std;

int main(int argc, char * argv[]){
  // process option
  // -r relevence feedback
  // -i input query file
  // -o ranked list
  // -m model.dir
  // -d NCTIR-dir
  string options, oprions_arg;
  bool relevence_feedback = false;
  string query_path = ""; 
  string rank_list_path = "";
  string model_path = ""; 
  string NCTIR_dir = ""; 
  for(size_t i = 1 ; i < argc ; ++i){
    if(strcmp(argv[i], "-r") == 0){
      relevence_feedback = true;
    }else if(strcmp(argv[i], "-i") == 0){
      query_path = argv[++i];
    }else if(strcmp(argv[i], "-o") == 0){
      rank_list_path = argv[++i];
    }else if(strcmp(argv[i], "-m") == 0){
      model_path = argv[++i];
    }else if(strcmp(argv[i], "-d") == 0){
      NCTIR_dir = argv[++i];
    }else{
      cout << "Invalid Options" << endl;
      exit(1);
    }
  }
  if(query_path == ""){
    cout << "query path -i is required" << endl;
    exit(1);
  }
  if(rank_list_path == ""){
    cout << "rank_list_path -o is required" << endl;
    exit(1);
  }
  if(model_path == ""){
    cout << "model_path -m is required" << endl;
    exit(1);
  }
  if(NCTIR_dir == ""){
    cout << "NCTIR_dir -d is required" << endl;
    exit(1);
  }
  if(relevence_feedback){
    cout << "relevence_feedback" << endl;
  }
  cout << "query_path: " << query_path << endl;
  cout << "rank_list_path: " << rank_list_path << endl;
  cout << "model_path: " << model_path << endl;
  cout << "NCTIR_dir: " << NCTIR_dir << endl;

  string file_list_path = model_path + "/file-list";
  DocContainer doc_container(file_list_path, NCTIR_dir);  
  cout << "Finish doc_container" << endl;

  // build string to id
  string voc_list_path = model_path + "/vocab.all";
  VocToIdMap voc_to_id_map(voc_list_path);
  cout << "Finish voc_to_id_map" << endl;

  // build inverted_file (index, List<file_info>)
  string inverted_file_path = model_path + "/inverted-file";
  InvertedFile inverted_file(inverted_file_path);
  cout << "Finish inverted_file" << endl;
  
  // set tf
  doc_container.set_docs_tf(inverted_file);
  cout << "Finish docs_tf" << endl;
  
  // set tf, idf policy
  TFInterface * tf = new OKapi(doc_container, inverted_file, voc_to_id_map);
  IDFInterface * idf = new IDF(doc_container, inverted_file);
  cout << "Finish initiate tf, idf" << endl;

  // query processing
  QueriesContainer queries_container;
  queries_container.parse(query_path, tf, idf);
  queries_container.get_result(inverted_file, doc_container, rank_list_path, relevence_feedback);
  cout << "Finish query processing" << endl;
}
