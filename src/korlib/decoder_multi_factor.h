// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#ifndef DECODERMULTIFACTOR_HPP_
#define DECODERMULTIFACTOR_HPP_

#include "common.h"
#include "decoder_base.h"
#include "ngram.h"
#include "factor_list.h"

namespace ngramchecker {


struct MorphoNode;
SP_DEF(MorphoNode);


class SimWordsFinder;
SP_DEF(SimWordsFinder);

class DecoderMultiFactor : public DecoderBase
{
 private:

  vector<FactorList> factorLists;
  NGram ngram_search_key;
  NGram ngram_val;
  SimWordsFinderP simWordsFinder;

  virtual vector<vector<StagePosibilityP> > init_inner_stage_posibilities(const vector<TokenP> &tokens);

  virtual double ComputeTransitionCost(ViterbiStateP state, StagePosibilityP next);

  virtual StagePosibilityP sentence_start_SP();
  virtual StagePosibilityP sentence_end_SP();


 public:
  virtual double ComputeTransitionCostSPSequence(vector<StagePosibilityP> &sp_vec, uint32_t start_index, uint32_t end_index);

  DecoderMultiFactor(Configuration *configuration);

};

}

#endif /* DECODERMULTIFACTOR_HPP_ */
