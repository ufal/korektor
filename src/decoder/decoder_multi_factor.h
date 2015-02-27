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
#include "decoder_base.h"
#include "language_model/ngram.h"
#include "morphology/factor_list.h"

namespace ufal {
namespace korektor {

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

  virtual vector<vector<StagePossibilityP> > init_inner_stage_posibilities(const vector<TokenP> &tokens);

  virtual double ComputeTransitionCost(ViterbiStateP state, StagePossibilityP next);

  virtual StagePossibilityP sentence_start_SP();
  virtual StagePossibilityP sentence_end_SP();


 public:
  virtual double ComputeTransitionCostSPSequence(vector<StagePossibilityP> &sp_vec, uint32_t start_index, uint32_t end_index);

  DecoderMultiFactor(Configuration *configuration);

};

} // namespace korektor
} // namespace ufal
