// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "stage_posibility.h"
#include "utils.h"
#include "configuration.h"

namespace ufal {
namespace korektor {

string StagePosibilityNew::ToString()
{
  return MyUtils::utf16_to_utf8(word);
}

bool StagePosibilityNew::IsUnknown()
{
  return form_id == MyConstants::unknown_word_id || form_id == MyConstants::name_id;
}

StagePosibilityNew::StagePosibilityNew(const FactorList &_factorList, bool _original, const u16string &_word, Configuration* _conf, float error_model_cost):
  word(_word), original(_original), emission_prob(error_model_cost), factorList(_factorList)
{

  uniq_id = 0;

  unsigned num_factors = _conf->NumFactors();

  for (unsigned j = 0; j < num_factors; j++)
  {
    if (_conf->IsFactorEnabled(j))
    {
      if (j > 0)
        emission_prob += _conf->GetFactorWeight(j) * factorList.emission_costs[j];
      MyUtils::HashCombine(uniq_id, factorList.factors[j]);
    }
  }

  form_id = factorList.factors[0];
}

} // namespace korektor
} // namespace ufal
