// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <unordered_set>

#include "common.h"
#include "tokenizer/token.h"
#include "viterbi_state.h"

namespace ufal {
namespace korektor {

class StagePossibility;
SP_DEF(StagePossibility);
typedef shared_ptr<vector<vector<StagePossibilityP>>> StagePossibilitiesType;

class TransitionCostComputation;
SP_DEF(TransitionCostComputation);

class Configuration;

struct Pair_StagePossibilityP_double_comparer : less<pair<uint32_t, double> >
{
  bool operator()(const pair<StagePossibilityP, double> &val1, const pair<StagePossibilityP, double> &val2)
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

  StagePossibilitiesType stage_posibilities;

  virtual vector<vector<StagePossibilityP> > init_inner_stage_posibilities(const vector<TokenP> &tokens) = 0;

  void init_posibilities(const vector<TokenP> &tokens);

  virtual double ComputeTransitionCost(ViterbiStateP state, StagePossibilityP next) = 0;

  virtual StagePossibilityP sentence_start_SP() = 0;
  virtual StagePossibilityP sentence_end_SP() = 0;

 protected:

  uint32_t viterbi_order;

  DecoderBase(Configuration* _configuration);

 public:

  virtual double ComputeTransitionCostSPSequence(vector<StagePossibilityP> &sp_vec, uint32_t start_index, uint32_t end_index) = 0;

  Configuration* configuration;

  uint32_t GetViterbiOrder();

  TransitionCostComputationP GetTransitionCostComputation();

  vector<StagePossibilityP>  DecodeTokenizedSentence(const vector<TokenP> &tokens);

  void DecodeTokenizedSentence_ReturnStagePossibilities(const vector<TokenP> &tokens, vector<StagePossibilityP> &decoded_sequence, StagePossibilitiesType &_stage_posibilities);

  string DecodeSentence(string sentence);

};

SP_DEF(DecoderBase);

} // namespace korektor
} // namespace ufal
