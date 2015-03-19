// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "spellchecker/configuration.h"
#include "stage_possibility.h"
#include "utils/hash.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

StagePossibility::StagePossibility(const FactorList& factor_list, bool original, const u16string& word, Configuration* conf, float error_model_cost) :
  word(word), original(original), emission_prob(error_model_cost), factor_list(factor_list)
{
  form_id = factor_list.factors[0];

  unique_id = 0;
  for (unsigned j = 0, num_factors = conf->NumFactors(); j < num_factors; j++)
  {
    if (conf->IsFactorEnabled(j))
    {
      if (j > 0)
        emission_prob += conf->GetFactorWeight(j) * factor_list.emission_costs[j];
      Hash::Combine(unique_id, factor_list.factors[j]);
    }
  }
}

} // namespace korektor
} // namespace ufal
