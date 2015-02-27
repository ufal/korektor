// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "spellchecker/configuration.h"
#include "spellchecker/constants.h"
#include "stage_possibility.h"
#include "utils/utf.h"
#include "utils/utils.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

string StagePossibilityNew::ToString()
{
  return UTF::UTF16To8(word);
}

bool StagePossibilityNew::IsUnknown()
{
  return form_id == Constants::unknown_word_id || form_id == Constants::name_id;
}

StagePossibilityNew::StagePossibilityNew(const FactorList &_factorList, bool _original, const u16string &_word, Configuration* _conf, float error_model_cost):
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
      Utils::HashCombine(uniq_id, factorList.factors[j]);
    }
  }

  form_id = factorList.factors[0];
}

} // namespace korektor
} // namespace ufal
