#include<fstream>
#include<iostream>
#include<vector>
#include<set>
#include<sstream>
#include<assert.h>
#include<string>

using namespace std;

class Collection{
public:
  Collection(string line){
    string delimeter = ",";
    size_t pos = line.find(delimeter);
    query_id = line.substr(0, pos);
    line.erase(0, pos+delimeter.length());
    istringstream is(line);
    string docs_id;
    while(is >> docs_id){
      docs_vec.push_back(docs_id);
    }
  }
  void print(){
    cout << "query_id: " << query_id << endl;
    for(size_t i = 0 ; i < docs_vec.size() ; ++i){
      cout << docs_vec[i] << " ";
    }
    cout << endl;
  }

  void make_set(){
    for(size_t i = 0 ; i < docs_vec.size() ; ++i){
      docs_set.insert(docs_vec[i]);
    }
  }

  bool exist(string docs_id){
    set<string>::iterator it = docs_set.find(docs_id);
    if(it == docs_set.end()) return false;
    return true;
  }
  string query_id;
  vector<string> docs_vec;
  set<string> docs_set;
};

double get_ap(Collection * ans, Collection * my){
  assert(ans->query_id == my->query_id);
  ans->make_set();
  vector<string>& docs_vec = my->docs_vec;
  size_t rel_cnt = 0;
  double ap = 0.0;
  for(size_t cnt = 1 ; cnt <= docs_vec.size() ; ++cnt){
    size_t index = cnt - 1;
    if(ans->exist(docs_vec[index])){
      rel_cnt += 1;
      cout << "Hit at round " << cnt << docs_vec[index] << endl;
      ap += (double)rel_cnt/(double)cnt;
    }
  }
  return ap/(double)ans->docs_vec.size();
}

int main(int argc, char ** argv){
  if(argc < 2){
    cout << "input csv is required" << endl;
    exit(1);
  }
  ifstream my_rank_list(argv[1]);
  if(!my_rank_list.is_open()){
    cout << "Cannot open file" << endl;
  }
  string line;
  vector<Collection *> my_rank_list_vec;
  while(getline(my_rank_list, line)){
    Collection * collection = new Collection(line);
    my_rank_list_vec.push_back(collection);
  }
  ifstream ans_rank_list("../queries/ans_train.csv");
  if(!ans_rank_list.is_open()){
    cout << "Cannot open file" << endl;
  }
  vector<Collection *> ans_rank_list_vec;
  while(getline(ans_rank_list, line)){
    Collection * collection = new Collection(line);
    ans_rank_list_vec.push_back(collection);
  }
  double map = 0;
  for(size_t i = 0 ; i < ans_rank_list_vec.size() ; ++i){
    double ap = get_ap(ans_rank_list_vec[i], my_rank_list_vec[i]);
    cout << ap << endl;
    map += ap;
  }
  cout << map/(double)ans_rank_list_vec.size() << endl;
}
