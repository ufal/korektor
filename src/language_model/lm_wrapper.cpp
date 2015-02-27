// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <iostream>

#include "lm_wrapper.h"
#include "zip_lm.h"

namespace ufal {
namespace korektor {

//!!! The ordering of IDs is not reversed anymore !!!
void LMWrapper::GetNGram(NGram& ngram_key, NGram& ngram_ret)
{
  LM->GetNGramForNGramKey(ngram_key, ngram_ret);

  uint32_t start_undef = ngram_ret.order + 1;
  uint32_t end_undef = ngram_key.order;

  for (uint32_t undef_order = start_undef; undef_order <= end_undef; undef_order++)
    ngram_key.order = undef_order;

  ngram_key.order = end_undef;
}

LMWrapper::LMWrapper(ZipLMP _LM):
  LM(_LM),
  _lookups(0),
  _hits(0)
{}

double LMWrapper::GetProb(NGram& ngram_key, NGram& ngram_pom)
{
  GetNGram(ngram_key, ngram_pom);

  double ret_prob = ngram_pom.prob;

  if (ngram_key.order == ngram_pom.order)
    return ret_prob;

  int highest_bow_order = ngram_key.order - 1;
  int lowest_bow_order = std::max(1, (int)ngram_pom.order);

  unsigned ngram_orig_order = ngram_key.order;

  ngram_key.word_ids++;
  ngram_key.order = (unsigned)highest_bow_order;

  while ((int)ngram_key.order >= lowest_bow_order)
  {
    GetNGram(ngram_key, ngram_pom);

    if ((int)ngram_pom.order >= lowest_bow_order)
      ret_prob += ngram_pom.backoff;

    if (ngram_pom.order == 0)
      break;

    ngram_key.order = ngram_pom.order - 1;
  }

  ngram_key.word_ids--;
  ngram_key.order = ngram_orig_order;

  return ret_prob;

}


bool LMWrapper::ContainsNGram(NGram& ngram_key, NGram& ngram_pom)
{
  GetNGram(ngram_key, ngram_pom);

  return (ngram_pom.order > 0);
}

void LMWrapper::PrintStats()
{
  cerr << "LMWrapper_ZipLM \"" << LM->GetFilename() << "\" Stats:" << endl << "lookups = " << _lookups << endl << "hits = " << _hits << endl;
}

uint32_t LMWrapper::MaxUnigramID()
{
  return LM->MaxUnigramID();
}

string LMWrapper::FactorName()
{
  return LM->GetFactorName();
}

} // namespace korektor
} // namespace ufal
