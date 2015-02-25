// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "viterbi_state.h"
#include "stage_posibility.h"
#include "utils.h"

namespace ngramchecker {

bool ViterbiState::CheckIt()
{

  for (uint32_t i = 0; i < history.size(); i++)
  {
    if (history[i]->type != type)
      return false;
  }

  return true;
}

StagePosibilityP ViterbiState::GetYoungestHistory()
{
  return history[history.size() - 1];
}

size_t ViterbiState::UniqueIdentifier()
{
  size_t seed = 0;

  MyUtils::HashCombine(seed, history.size());

  for (uint32_t i = 0; i < history.size(); i++)
  {
    MyUtils::HashCombine(seed, history[i]->UniqueIdentifier());
  }

  return seed;

}

bool ViterbiState::Equals(ViterbiState &state)
{
  return this->UniqueIdentifier() == state.UniqueIdentifier();
}

ViterbiState::ViterbiState(vector<StagePosibilityP> _history):
  history(_history), distance(0), ancestor(ViterbiStateP())
{
  type = _history[0]->type;
  assert(CheckIt());
}

ViterbiState::ViterbiState(ViterbiStateP prev_state, StagePosibilityP next_sp, double _distance):distance(_distance), ancestor(prev_state)
{
  type = next_sp->type;

  history.reserve(prev_state->history.size());

  for (size_t i = 1; i < prev_state->history.size(); i++)
  {
    history.push_back(prev_state->history[i]);
  }

  history.push_back(next_sp);

  //distance = _distance;

  //ancestor = prev_state;
  assert(CheckIt());
}

string ViterbiState::ToString()
{
  stringstream strs;
  strs << "(Old -> Young): ";

  for (uint32_t i = 0; i < history.size(); i++)
  {
    if (i > 0) strs << ", ";
    strs << history[i]->ToString();
  }

  return strs.str();
}
}
