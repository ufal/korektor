/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#ifndef VITERBISTATE_HPP_
#define VITERBISTATE_HPP_

#include "StdAfx.h"

namespace ngramchecker {

class ViterbiState;
SP_DEF(ViterbiState);

class StagePosibility;
SP_DEF(StagePosibility);


//TODO: change the structure of ViterbiState
//template parameters: HISTORY_LENGTH, NUM_FACTORS
//ngram_history_struct history[NUM_FACTORS][HISTORY_LENGTH]
//

struct ngram_history_struct
{
  //word ids are actually not needed!
  uint32_t zipTBO_index; //index into the corresponding level of ZipTBO structures
  float bow; //acumulated BOW - i.e. already compensates for bow at higher levels
  float prob;
};

class ViterbiState
{
 public:
  StagePosibility_type type;

  vector<StagePosibilityP> history;

  double distance;
  ViterbiStateP ancestor;

 private:

  bool CheckIt();

 public:

  StagePosibilityP GetYoungestHistory();

  size_t UniqueIdentifier();

  bool Equals(ViterbiState &state);

  ViterbiState(vector<StagePosibilityP> _history);

  ViterbiState(ViterbiStateP prev_state, StagePosibilityP next_sp, double _distance);

  string ToString();
};




struct ViterbiStateP_compare_distance : std::less<ViterbiStateP>
{
  bool operator()(const ViterbiStateP& s1, const ViterbiStateP& s2)
  {
    return s1->distance < s2->distance;
  }
};

struct ViterbiStateP_iequal_to
: std::binary_function<ViterbiStateP, ViterbiStateP, bool>
{
  bool operator()(ViterbiStateP const& x,
                  ViterbiStateP const& y) const
  {
    return x->Equals(*y);
  }
};


struct ViterbiStateP_ihash: std::unary_function<ViterbiStateP, std::size_t>
{
  std::size_t operator()(ViterbiStateP const& x) const
  {
    return x->UniqueIdentifier();
  }
};



}

#endif /* VITERBISTATE_HPP_ */
