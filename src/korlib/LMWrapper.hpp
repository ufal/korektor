/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#ifndef LMWRAPPER_HPP_
#define LMWRAPPER_HPP_

#include "StdAfx.h"
#include "NGram.hpp"
#include "MyCash.hpp"
#include "ZipLM.hpp"

namespace ngramchecker {

struct identity_hash : std::unary_function<size_t, size_t>
{
  size_t operator()(size_t val) const
  {
    return val;
  }
};

//this is ok, cause the key is already a hash value...
//TODO: change the value type to triple(uint, uint8_t, uint8_t) and use a global probabilistic mapping from uint8_t to float!
typedef MyCash<size_t, triple(uint, double, double)/*, identity_hash*/> MyCash_NGram;

//TODO: change the value type from double to uint8_t and do the same as before
typedef MyCash<size_t, double/*, identity_hash*/> MyCash_NGramProbs;


//typedef MyCash<NGramP, NGramP, NGramP_ihash, NGramP_iequal_to> MyCash_NGramP;
//typedef MyCash<NGramP, double, NGramP_ihash, NGramP_iequal_to> MyCash_NGramProbs;

class LMWrapper {
 private:
  ZipLMP LM;

  uint32_t _lookups;
  uint32_t _hits;

  MyCash_NGram ngram_cash;
  MyCash_NGramProbs ngram_probs_cash;

  void GetNGram(NGram &ngram_key, NGram &ngram_val);

  static NGram_ihash ngram_hash_function;

 public:

  LMWrapper(ZipLMP _LM, uint32_t ngram_cash_capacity, uint32_t ngram_probs_cash_capacity);

  string FactorName();

  //gets the probability of any ngram. 'ngram_key' is supposed to have the word ids set according, ngram_pom is an auxiliary variable used during the computation
  //this function uses the back-off strategy (i.e. using lower order ngrams when the higher order ngram wasn't found
  double GetProb(NGram &ngram_key, NGram &ngram_pom);

  void ClearCash();

  //Whether the language model contains the specified ngram (if it contains only lower order ngrams, then the result is false)
  //ngram_key should have word_ids initialized, ngram_pom is an auxiliary variable
  bool ContainsNGram(NGram &ngram_key, NGram &ngram_pom);
  void PrintStats();
  uint32_t MaxUnigramID();

  bool getFirstLevelTuple(uint word_id, LM_tuple &ret) { return LM->getFirstLevelTuple(word_id, ret); }
  bool GetTuple(uint level, uint word_id, uint offset, uint num_entries, LM_tuple &lm_tuple) { return LM->GetTuple(level, word_id, offset, num_entries, lm_tuple); }

};

SP_DEF(LMWrapper);

}

#endif /* LMWRAPPER_HPP_ */
