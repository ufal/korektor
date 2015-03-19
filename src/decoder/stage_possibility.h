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
#include "morphology/factor_list.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

struct StagePossibility {
  u16string word;
  bool original;
  float emission_prob;
  FactorList factor_list;
  uint32_t form_id;
  size_t unique_id;

  StagePossibility(const FactorList& factor_list, bool original, const u16string& word, Configuration* conf, float error_model_cost);
};
SP_DEF(StagePossibility);

typedef shared_ptr<vector<vector<StagePossibilityP>>> StagePossibilitiesType;

struct StagePossibility_Form_comparer : less<StagePossibilityP> {
  bool operator()(const StagePossibilityP& sp1, const StagePossibilityP& sp2)
  {
    return sp1->form_id  < sp2->form_id;
  }
};

struct StagePossibility_sort_cost : less<StagePossibilityP> {
  bool operator()(const StagePossibilityP& sp1, const StagePossibilityP& sp2)
  {
    if (sp1->emission_prob < sp2->emission_prob) return true;
    if (sp1->emission_prob > sp2->emission_prob) return false;
    return sp1->unique_id < sp2->unique_id;
  }
};

} // namespace korektor
} // namespace ufal
