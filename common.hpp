#ifndef COMMON_HPP
#define COMMON_HPP
class GramsInfo{
public:
  GramsInfo(size_t grams_id, double weight): grams_id(grams_id), weight(weight){}
  size_t grams_id;
  double weight;
};
#endif
