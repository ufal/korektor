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
#include "viterbi_state.h"
#include "token.h"

namespace ngramchecker {


class StagePosibility;
SP_DEF(StagePosibility);

class TransitionCostComputation;
SP_DEF(TransitionCostComputation);

class Configuration;

typedef unordered_set<ViterbiStateP, ViterbiStateP_ihash, ViterbiStateP_iequal_to> Trellis_stage_set;

typedef vector<shared_ptr<ViterbiState> > Trellis_stage;
SP_DEF(Trellis_stage);
SP_DEF(Trellis_stage_set);

typedef shared_ptr<vector<vector<StagePosibilityP> > > StagePosibilitiesType;

struct Pair_StagePosibilityP_double_comparer : less<pair<uint32_t, double> >
{
  bool operator()(const pair<StagePosibilityP, double> &val1, const pair<StagePosibilityP, double> &val2)
  {
    return val1.second < val2.second;
  }
};

/// @brief Class for finding the spelling corrected sentence
///
/// Decoder is responsible for finding the solution (i.e. the corrected sentence)
/// The steps are:
/// 1) similar words to the input tokens are found
/// 2) All the candidate words from step 1) are morphologically analysed
/// (after step 1) and 2), we have lists of possible corrections for each input token
/// 3) Decoding of the sentence
/// 4) Ordering the suggestion list for each mispelled word found
class DecoderBase
{

 private:

  StagePosibilitiesType stage_posibilities;

  virtual vector<vector<StagePosibilityP> > init_inner_stage_posibilities(const vector<TokenP> &tokens) = 0;

  void init_posibilities(const vector<TokenP> &tokens);

  virtual double ComputeTransitionCost(ViterbiStateP state, StagePosibilityP next) = 0;

  virtual StagePosibilityP sentence_start_SP() = 0;
  virtual StagePosibilityP sentence_end_SP() = 0;

 protected:

  uint32_t viterbi_order;

  DecoderBase(Configuration* _configuration);

 public:

  virtual double ComputeTransitionCostSPSequence(vector<StagePosibilityP> &sp_vec, uint32_t start_index, uint32_t end_index) = 0;

  Configuration* configuration;

  uint32_t GetViterbiOrder();

  TransitionCostComputationP GetTransitionCostComputation();

  vector<StagePosibilityP>  DecodeTokenizedSentence(const vector<TokenP> &tokens);

  void DecodeTokenizedSentence_ReturnStagePosibilities(const vector<TokenP> &tokens, vector<StagePosibilityP> &decoded_sequence, StagePosibilitiesType &_stage_posibilities);

  string DecodeSentence(string sentence);

};

SP_DEF(DecoderBase);

}
