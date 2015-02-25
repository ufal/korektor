// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"
#include "ngram.h"
#include "zip_lm.h"

namespace ngramchecker {

struct identity_hash : std::unary_function<size_t, size_t>
{
  size_t operator()(size_t val) const
  {
    return val;
  }
};


class LMWrapper {
 private:
  ZipLMP LM;

  uint32_t _lookups;
  uint32_t _hits;

  void GetNGram(NGram &ngram_key, NGram &ngram_val);

  static NGram_ihash ngram_hash_function;

 public:

  LMWrapper(ZipLMP _LM);

  string FactorName();

  //gets the probability of any ngram. 'ngram_key' is supposed to have the word ids set according, ngram_pom is an auxiliary variable used during the computation
  //this function uses the back-off strategy (i.e. using lower order ngrams when the higher order ngram wasn't found
  double GetProb(NGram &ngram_key, NGram &ngram_pom);

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
