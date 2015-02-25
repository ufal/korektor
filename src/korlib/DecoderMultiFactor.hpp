/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#ifndef DECODERMULTIFACTOR_HPP_
#define DECODERMULTIFACTOR_HPP_

#include "StdAfx.h"
#include "DecoderBase.hpp"
#include "NGram.hpp"
#include "FactorList.hpp"

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
