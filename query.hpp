#ifndef QUERY_HPP
#define QUERY_HPP

#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include "vsm.hpp"
#include "common.hpp"

#ifndef TOP_K
#define TOP_K 100
#endif

using namespace std;

class Query{
  public:
  Query(string number, string title, string question, string narrative, string concepts, 
        TFInterface * tf, IDFInterface * idf)
  { 
    this->id = fromStringToId(number);
    this->title = title;
    this->question = question;
    this->narrative = narrative;
    this->tf = tf;
    this->idf = idf;
    concepts_to_vector(concepts);
  }

  string fromStringToId(string number);
  void concepts_to_vector(string concepts);
  void get_result(InvertedFile&, DocContainer&, ofstream& rank_list_path, bool relevence_feedback);
  void relevence_feedback(vector<pair<size_t, double> >& topk_doc_vec, DocContainer& doc_container);
  string id;
  string title;
  string question;
  string narrative;
  vector<GramsInfo *> term_vec;
  TFInterface * tf;
  IDFInterface * idf;
};

class QueriesContainer{
  public:
     vector<Query *> query_list;
  void parse(string query_path, TFInterface *, IDFInterface *);
  void get_result(InvertedFile&, DocContainer&, string rank_list_path, bool relevence_feedback);
  string parseTag(ifstream& xml_file);
};

#endif
