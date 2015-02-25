/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file zip_lm.h
/// @brief Class for creating, manipulating language models
/// @class ZipLM zip_lm.h "zip_lm.h"

#ifndef ZIPLM_HPP_
#define ZIPLM_HPP_

#include "common.h"

namespace ngramchecker {

class MyPackedArray;
SP_DEF(MyPackedArray);

class MyMappedDoubleArray;
SP_DEF(MyMappedDoubleArray);

class CompIncreasingArray;
SP_DEF(CompIncreasingArray);

class NGram;
SP_DEF(NGram);

class Morphology;
SP_DEF(Morphology);

//LM_tuple represents a node of a language model tree, it contains the extent of the higher level ngrams (next level child nodes) by 'nlevel_offset' and 'nlevel_entries'
//, it also stores ngram probability 'prob' and back-off weight 'bow'
struct LM_tuple {
  //uint32_t word_id;
  uint nlevel_offset;
  int nlevel_entries;

  float prob;
  float bow;

};

class ZipLM;
SP_DEF(ZipLM);

class ZipLM {
 private:

  static const uint32_t bits_per_unigram_prob;
  static const uint32_t bits_per_unigram_bow;
  static const uint32_t bits_per_bigram_prob;
  static const uint32_t bits_per_bigram_bow;
  static const uint32_t bits_per_higher_order_prob;
  static const uint32_t bits_per_higher_order_bow;


  vector<MyMappedDoubleArrayP> probs;
  vector<MyMappedDoubleArrayP> bows;
  vector<MyPackedArrayP> ids;

  vector<CompIncreasingArrayP> offsets;

  vector<uint32_t> bits_per_probs;
  vector<uint32_t> bits_per_bows;

  uint32_t lm_order;
  double not_in_lm_cost;
  string filename;

  string factor_name;

  uint32_t max_unigram_id;

  int search_for_id(uint32_t _order, uint32_t word_id, int start_offset, int end_offset);

  void get_lm_tuple(uint32_t _order, uint32_t _offset, LM_tuple &ret);

  //void GetAllNGramsPerUnigramID_impl(uint32_t order, int start_index, int end_index, vector<uint32_t> lo_ids, vector<NGramP> &ret);

 public:

  bool getFirstLevelTuple(uint word_id, LM_tuple &ret);

  //searching for a child node, given the LM tree level, word_id of the child node. 'offset' and 'num_entries' specify the extent of searching
  //within the array storing the nodes for the given tree level.
  bool GetTuple(uint level, uint word_id, uint offset, uint num_entries, LM_tuple &lm_tuple);


  string GetFactorName() { return factor_name; }

  //vector<NGramP> GetAllNGramsPerUnigramID(uint32_t unigramID);

  string GetFilename();

  double GetWordNotInLMCost();

  //returns the longest matching ngram. I.e. if ngram corresponding to 'ngram_key' is not found, shorter ngram with matching word ids can be returned
  void GetNGramForNGramKey(NGram& ngram_key, NGram& ngram_ret);

  ZipLM(const string& _factor_name, uint32_t _order, double _not_in_lm_cost, vector<vector<double> > &_probs, vector<vector<double> > &_bows,
        vector<vector<uint32_t> > &_ids, vector<vector<uint32_t> > &_offsets);

  ZipLM(string bin_file);
  //ZipLM(const vector<NGramP> &ngrams);
  void SaveInBinaryForm(string out_file);

  //creates the language model instance from the ARPA format language model stored in 'text_file'
  static ZipLMP createFromTextFile(string text_file, MorphologyP &morphology, string _factor_name, uint lm_order, double not_in_lm_cost);

  uint32_t MaxUnigramID();

  void TestIt(string textFile);
};

}

#endif // ZIPLM_HPP_
