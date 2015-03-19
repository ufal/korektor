// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <algorithm>

#include "decoder_multi_factor.h"
#include "error_model/error_model.h"
#include "language_model/lm_wrapper.h"
#include "language_model/ngram.h"
#include "lexicon/sim_words_finder.h"
#include "morphology/morphology.h"
#include "spellchecker/configuration.h"
#include "spellchecker/constants.h"
#include "stage_possibility.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

double DecoderMultiFactor::ComputeTransitionCost(ViterbiStateP state, StagePossibilityP next)
{
  double ret_cost = 0;

  //TODO: is this n-gram ordering what we want???

  factorLists[viterbi_order - 1] = next->factor_list;

  for (unsigned i = 0; i < state->history.size(); i++)
    factorLists[i] = state->history[i]->factor_list;

  unsigned num_factors = configuration->NumFactors();

  for (int i = 0; i < (int)num_factors; i++)
  {
    if (configuration->FactorIsEnabled(i) == false) continue;

    for (unsigned j = 0; j < viterbi_order; j++)
    {
      //changing the word id ordering:
      //ngram_search_key.word_ids[j] = morpho_nodes[j]->factorID;
      ngram_search_key.word_ids[viterbi_order - 1 - j] = factorLists[j].factors[i];
    }

    ////changing the word id ordering:
    //ngram_search_key.word_ids += viterbi_order - ngram_search_key.order;
    ngram_search_key.order = configuration->GetFactorOrder(i);

    double cost = configuration->GetFactorWeight(i) * configuration->GetFactorLM(i)->GetProb(ngram_search_key, ngram_val);
    ret_cost += cost;

    if (configuration->diagnostics)
    {
      cout << "factor " << i << ", LM cost: (";
      for (unsigned j = 0; j < ngram_search_key.order; j++)
      {
        if (j > 1 && j < ngram_search_key.order) cout << " ";
        else if (j == 1) cout << " | ";

        cout << configuration->morphology->GetFactorString(i, ngram_search_key.word_ids[j]);
      }
      cout << ") ~ " << cost << endl;
    }

    ngram_search_key.word_ids -= viterbi_order - ngram_search_key.order;
  }

  if (configuration->diagnostics)
    cout << "total cost = " << ret_cost << endl;

  return ret_cost;
}

double DecoderMultiFactor::ComputeTransitionCostSPSequence(vector<StagePossibilityP> &sp_vec, uint32_t start_index, uint32_t end_index)
{
  double ret_cost = 0;
  uint32_t order = end_index - start_index + 1;

  for (int i = 0; i < (int)order; i++)
  {
    //The ordering of ngrams was changed here!!!
    factorLists[i] = sp_vec[i + start_index]->factor_list;
  }

  unsigned num_factors = configuration->NumFactors();

  for (unsigned h = 0; h < num_factors; h++)
  {
    if (configuration->FactorIsEnabled(h) == false)
      continue;

    ngram_search_key.order = configuration->GetFactorOrder(h);

    for (unsigned i = 0; i < order; i++)
    {
      ngram_search_key.word_ids[order - 1 - i] = factorLists[i].factors[h];
    }

    ngram_search_key.order = configuration->GetFactorOrder(h);

    ret_cost += configuration->GetFactorWeight(h) * configuration->GetFactorLM(h)->GetProb(ngram_search_key, ngram_val);
  }

  return ret_cost;

}

StagePossibilityP DecoderMultiFactor::sentence_start_SP()
{
  vector<FactorList> mnodes = configuration->morphology->GetMorphology(Constants::sentence_start_id, configuration);
  assert(mnodes.size() == 1);

  return StagePossibilityP(new StagePossibility(mnodes[0], true, UTF::UTF8To16("<s>"), configuration, 0.0));
}

StagePossibilityP DecoderMultiFactor::sentence_end_SP()
{
  vector<FactorList> mnodes = configuration->morphology->GetMorphology(Constants::sentence_end_id, configuration);
  assert(mnodes.size() == 1);

  return StagePossibilityP(new StagePossibility(mnodes[0], true, UTF::UTF8To16("</s>"), configuration, 0.0));
}


vector<vector<StagePossibilityP> > DecoderMultiFactor::init_inner_stage_posibilities(const vector<TokenP> &tokens)
{
  vector<vector<StagePossibilityP> > ret;

  if (tokens.size() == 0)
    return ret;

  for (uint32_t i = 0; i < tokens.size(); i++)
  {
    u16string& u_word = tokens[i]->str;

    vector<StagePossibilityP> vec_stage_pos;


    {
      Similar_Words_Map msw = configuration->simWordsFinder->Find(tokens[i]);


      for (Similar_Words_Map::iterator it = msw.begin(); it != msw.end(); it++)
      {
        uint32_t sim_word_id = it->first;
        u16string sim_word_str = it->second.first;
        bool is_original = configuration->errorModel->StringsAreIdentical(u_word, sim_word_str);
        double err_model_cost = it->second.second;

        vector<FactorList> f_lists = configuration->morphology->GetMorphology(sim_word_id, configuration);

        for (auto it2 = f_lists.begin(); it2 != f_lists.end(); it2++)
        {
          StagePossibilityP spp = StagePossibilityP(new StagePossibility(*it2, is_original, sim_word_str, configuration, err_model_cost));
          vec_stage_pos.push_back(spp);
        }

      }

      if (vec_stage_pos.empty())
      {
        vector<FactorList> mn = configuration->morphology->GetMorphology(Constants::unknown_word_id, configuration);
        assert(mn.size() == 1);

        StagePossibilityP spp = StagePossibilityP(new StagePossibility(mn[0], true, u_word, configuration, configuration->errorModel->UnknownWordCost()));
        vec_stage_pos.push_back(spp);
      }

      std::sort(vec_stage_pos.begin(), vec_stage_pos.end(), StagePossibility_sort_cost());
    }
    ret.push_back(vec_stage_pos);
  }

  return ret;
}

DecoderMultiFactor::DecoderMultiFactor(Configuration* _configuration):
  DecoderBase(_configuration)
{
  ngram_search_key = NGram(viterbi_order);
  ngram_val = NGram(this->viterbi_order);
  factorLists.resize(viterbi_order);

}

} // namespace korektor
} // namespace ufal
