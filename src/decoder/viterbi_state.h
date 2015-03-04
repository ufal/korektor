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

namespace ufal {
namespace korektor {

class StagePossibility;
SP_DEF(StagePossibility);

class ViterbiState;
SP_DEF(ViterbiState);

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
  vector<StagePossibilityP> history;

  double distance;
  ViterbiStateP ancestor;
  size_t unique_identifier;

  StagePossibilityP GetYoungestHistory();

  bool Equals(ViterbiState &state);

  ViterbiState(vector<StagePossibilityP> _history);

  ViterbiState(ViterbiStateP prev_state, StagePossibilityP next_sp, double _distance);

  string ToString();

 private:
  void ComputeUniqueIdentifier();
};




struct ViterbiStateP_compare_distance : std::less<ViterbiStateP>
{
  bool operator()(const ViterbiStateP& s1, const ViterbiStateP& s2)
  {
    if (s1->distance < s2->distance) return true;
    if (s1->distance > s2->distance) return false;
    return s1->unique_identifier < s2->unique_identifier;
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
    return x->unique_identifier;
  }
};

} // namespace korektor
} // namespace ufal
