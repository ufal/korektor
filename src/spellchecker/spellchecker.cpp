// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file spellchecker.cpp
/// @brief Main spellchecker source code

#include <algorithm>
#include <queue>
#include <set>
#include <sstream>

#include "configuration.h"
#include "constants.h"
#include "decoder/decoder_multi_factor.h"
#include "decoder/stage_possibility.h"
#include "lexicon/sim_words_finder.h"
#include "morphology/morphology.h"
#include "spellchecker.h"
#include "token/token.h"

namespace ufal {
namespace korektor {

void Spellchecker::Spellcheck(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned alternatives) {
  // Run Viterbi
  vector<StagePossibilityP> decoded_corrections;
  StagePossibilitiesType decoded_alternatives;
  decoder->DecodeTokenizedSentence_ReturnStagePossibilities(tokens, decoded_corrections, decoded_alternatives);

  // Data structures for alternatives selection
  struct AlternativeWithCost {
    double cost;
    const StagePossibility* alternative;

    bool operator<(const AlternativeWithCost& other) const { return cost < other.cost; }
    AlternativeWithCost(double cost, StagePossibility* alternative) : cost(cost), alternative(alternative) {}
  };
  unordered_map<uint32_t, AlternativeWithCost> alternatives_cost;
  priority_queue<AlternativeWithCost> alternatives_heap;

  // Clean corrections
  corrections.clear();
  corrections.reserve(tokens.size());

  // Fill corrections
  unsigned viterbi_order = decoder->GetViterbiOrder();
  for (unsigned i = 0; i < tokens.size(); i++) {
    auto correction = decoded_corrections[i + viterbi_order - 1];
    if (correction->original) {
      corrections.emplace_back(SpellcheckerCorrection::NONE);
    } else {
      corrections.emplace_back(tokens[i]->isUnknown() ? SpellcheckerCorrection::SPELLING : SpellcheckerCorrection::GRAMMAR);
      corrections.back().correction = correction->word;

      // Add alternatives if requested
      if (alternatives) {
        alternatives_cost.reserve(8 + 2 * decoded_alternatives->size());

        // Measure costs of alternative forms
        for (auto&& alternative : decoded_alternatives->at(i + viterbi_order - 1)) {
          if (alternative->form_id == correction->form_id) continue;
          auto same_alternative = alternatives_cost.find(alternative->form_id);
          bool have_current_best = same_alternative != alternatives_cost.end() || alternatives_cost.size() == alternatives;
          double current_best = same_alternative != alternatives_cost.end() ? same_alternative->second.cost :
              !alternatives_heap.empty() ? alternatives_heap.top().cost : 0;

          // Measure emmision probability
          double cost = alternative->emission_prob;
          if (have_current_best && cost > current_best) continue;

          // Measure transition costs of the alternative
          decoded_corrections[i + viterbi_order - 1] = alternative;
          for (unsigned k = 0; k < viterbi_order && i + k < tokens.size() + 1/*</s>*/; k++) {
            cost += decoder->ComputeTransitionCostSPSequence(decoded_corrections, i + k, i + viterbi_order - 1 + k);
            if (have_current_best && cost > current_best) break;
          }

          // Keep alternative if good enough
          if (same_alternative != alternatives_cost.end()) {
            if (cost < current_best) {
              // Lower the cost in alternative_costs
              same_alternative->second.cost = cost;
              same_alternative->second.alternative = alternative.get();
              alternatives_heap.push(same_alternative->second);
              // Normalize heap, i.e. remove dead top elements
              if (alternatives_heap.top().alternative->form_id == alternative->form_id) {
                alternatives_heap.pop();
                while (!alternatives_heap.empty() && alternatives_cost.at(alternatives_heap.top().alternative->form_id) < alternatives_heap.top())
                  alternatives_heap.pop();
              }
            }
          } else {
            if (alternatives_cost.size() < alternatives || cost < current_best) {
              if (alternatives_cost.size() >= alternatives) {
                alternatives_cost.erase(alternatives_heap.top().alternative->form_id);
                alternatives_heap.pop();
                // Normalize heap, i.e. remove dead top elements
                while (!alternatives_heap.empty() && alternatives_cost.at(alternatives_heap.top().alternative->form_id) < alternatives_heap.top())
                  alternatives_heap.pop();
              }
              alternatives_cost.emplace(alternative->form_id, AlternativeWithCost(cost, alternative.get()));
              alternatives_heap.emplace(cost, alternative.get());
            }
          }
        }
        decoded_corrections[i + viterbi_order - 1] = correction;

        // Store best alternatives
        corrections.back().alternatives.resize(alternatives_cost.size());
        for (unsigned i = alternatives_cost.size(); i > 0; i--) {
          corrections.back().alternatives[i - 1] = alternatives_heap.top().alternative->word;
          alternatives_cost.erase(alternatives_heap.top().alternative->form_id);
          alternatives_heap.pop();
          if (i > 1)
            // Normalize heap, i.e. remove dead top elements
            while (!alternatives_heap.empty() && alternatives_cost.at(alternatives_heap.top().alternative->form_id)< alternatives_heap.top())
              alternatives_heap.pop();
        }
        assert(alternatives_heap.empty());
      }
    }
  }
}

void Spellchecker::SpellcheckToken(const TokenP& token, SpellcheckerCorrection& correction, unsigned alternatives) {
  // Data structure for best correction selection
  struct CorrectionWithCost {
    double cost;
    u16string correction;

    bool operator<(const CorrectionWithCost& other) const { return cost < other.cost; }
    CorrectionWithCost(double cost, const u16string& correction) : cost(cost), correction(correction) {}
  };
  priority_queue<CorrectionWithCost> corrections;

  auto similar_words_map = configuration->simWordsFinder->Find(token);
  for (auto&& similar_word : similar_words_map) {
    uint32_t formID = similar_word.first;

    // Find best analysis of this form
    double best_analysis_cost = -1;
    for (auto&& factor : configuration->morphology->GetMorphology(formID, configuration)) {
      double cost = 0;
      for (unsigned i = 1; i < configuration->NumFactors(); i++)
        if (configuration->IsFactorEnabled(i))
          cost += configuration->GetFactorWeight(i) * factor.emission_costs[i];

      if (cost < best_analysis_cost || best_analysis_cost < 0) best_analysis_cost = cost;
    }

    // If there was any alternative
    if (best_analysis_cost >= 0) {
      double cost = similar_word.second.second + best_analysis_cost;

      if (corrections.size() < alternatives + 1 || cost < corrections.top().cost) {
        if (corrections.size() == alternatives + 1) corrections.pop();
        corrections.emplace(cost, similar_word.second.first);
      }
    }
  }

  // Fill the correction information
  correction.type = SpellcheckerCorrection::NONE;
  if (!corrections.empty()) {
    // Fill out correction.correction and correction.alternatives
    correction.alternatives.resize(corrections.size() - 1);
    for (unsigned i = corrections.size() - 1; i; i--) {
      correction.alternatives[i - 1] = corrections.top().correction;
      corrections.pop();
    }

    // Now we have access to the best correction, fill correct correction.type
    if (corrections.top().correction != token->str) {
      correction.correction = corrections.top().correction;
      correction.type = token->isUnknown() ? SpellcheckerCorrection::SPELLING : SpellcheckerCorrection::GRAMMAR;
    }
  }
}

Spellchecker::Spellchecker(Configuration* _configuration):
  configuration(_configuration), decoder(new DecoderMultiFactor(_configuration))
{}

} // namespace korektor
} // namespace ufal
