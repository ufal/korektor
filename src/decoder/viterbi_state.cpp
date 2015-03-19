// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <sstream>

#include "stage_possibility.h"
#include "utils/hash.h"
#include "utils/utf.h"
#include "viterbi_state.h"

namespace ufal {
namespace korektor {

StagePossibilityP ViterbiState::GetYoungestHistory()
{
  return history[history.size() - 1];
}

bool ViterbiState::Equals(ViterbiState &state)
{
  return this->unique_identifier == state.unique_identifier;
}

ViterbiState::ViterbiState(vector<StagePossibilityP> _history):
  history(_history), distance(0), ancestor(ViterbiStateP())
{
  ComputeUniqueIdentifier();
}

ViterbiState::ViterbiState(ViterbiStateP prev_state, StagePossibilityP next_sp, double _distance):distance(_distance), ancestor(prev_state)
{
  history.reserve(prev_state->history.size());

  for (size_t i = 1; i < prev_state->history.size(); i++)
  {
    history.push_back(prev_state->history[i]);
  }

  history.push_back(next_sp);

  ComputeUniqueIdentifier();
}

string ViterbiState::ToString()
{
  stringstream strs;
  strs << "(Old -> Young): ";

  for (uint32_t i = 0; i < history.size(); i++)
  {
    if (i > 0) strs << ", ";
    strs << UTF::UTF16To8(history[i]->word);
  }

  return strs.str();
}

void ViterbiState::ComputeUniqueIdentifier() {
  unique_identifier = 0;
  Hash::Combine(unique_identifier, history.size());
  for (auto&& h : history)
    Hash::Combine(unique_identifier, h->unique_id);
}

} // namespace korektor
} // namespace ufal
