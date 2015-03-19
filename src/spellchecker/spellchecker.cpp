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
#include "text_checking_result.h"
#include "token/tokenizer.h"
#include "utils/u16string_replacing.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

map<uint32_t, vector<StagePossibilityP> > Spellchecker::MakeSuggestionList(vector<StagePossibilityP> &decoded_pos, StagePossibilitiesType stage_posibilities) {


  //cerr << "Make suggestion started..." << endl;
  uint32_t viterbi_order = decoder->GetViterbiOrder();

  assert(stage_posibilities->size() == decoded_pos.size());


  map<uint32_t, vector<StagePossibilityP> > suggestions;


  for (uint32_t i = viterbi_order - 1; i < decoded_pos.size() - 1; i++)
  {
    //cerr << "i = " << i << endl;

    if (decoded_pos[i]->IsOriginal())
    {
      if (decoded_pos[i]->IsUnknown())
      {
        suggestions[i] = vector<StagePossibilityP>();
      }

      continue;
    }


    //assert (i < stage_posibilities->size());
    //assert (stage_posibilities != NULL);
    vector<StagePossibilityP> st_pos = stage_posibilities->operator [](i);

    vector<pair<StagePossibilityP, double> > cost_vector;

    suggestions[i] = vector<StagePossibilityP>();

    StagePossibilityP decoded_pos_at_i_backup = decoded_pos[i];

    for (uint32_t j = 0; j < st_pos.size(); j++)
    {
      //cerr << "j = " << j << endl;
      StagePossibilityP sp = st_pos[j];
      double cost = sp->EmmisionProbability();

      decoded_pos[i] = sp;

      for (uint32_t k = 0; k < viterbi_order; k++)
      {
        if (i + k >= decoded_pos.size() )
          break;

        cost += decoder->ComputeTransitionCostSPSequence(decoded_pos, i - viterbi_order + 1 + k, i + k);
      }

      cost_vector.push_back(pair<StagePossibilityP, double>(sp, cost));

    }

    decoded_pos[i] = decoded_pos_at_i_backup;

    std::sort(cost_vector.begin(), cost_vector.end(), Pair_StagePossibilityP_double_comparer());
    vector<pair<StagePossibilityP, double> > new_cost_vector;
    set<StagePossibilityP, StagePossibility_Form_comparer> word_ids_in_list;

    for (uint32_t j = 0; j < cost_vector.size(); j++)
    {
      if (cost_vector[j].first->IsOriginal())
        break;

      if (word_ids_in_list.find(cost_vector[j].first) == word_ids_in_list.end())
      {
        word_ids_in_list.insert(cost_vector[j].first);
        new_cost_vector.push_back(cost_vector[j]);
      }
    }


    for (vector<pair<StagePossibilityP, double> >::iterator it = new_cost_vector.begin(); it != new_cost_vector.end(); it++)
    {
      suggestions[i].push_back(it->first);
    }


  }

  //cerr << "... make suggestion finished!" << endl;
  return suggestions;

}

struct pair_u16string_double_comparer : public less<pair<u16string, double>>
{
  bool operator()(const pair<u16string, double> &a, const pair<u16string, double> &b)
  {
    return a.second < b.second;
  }
};

vector<string> Spellchecker::GetContextFreeSuggestions(const string &word)
{
  u16string u_word = UTF::UTF8To16(word);

  TokenP token(new Token(u_word));
  token->correction_is_allowed = true;
  token->sentence_start = false;

  Similar_Words_Map swm = configuration->simWordsFinder->Find(token);

  vector<pair<u16string, double> > word_cost_vec;

  for (Similar_Words_Map::iterator it = swm.begin(); it != swm.end(); it++)
  {

    uint32_t formID = it->first;
    double cost = it->second.second;
    auto& sim_word = it->second.first;

    vector<FactorList> flist = configuration->morphology->GetMorphology(formID, configuration);

    double best_em_cost = 100000;

    for (auto it = flist.begin(); it != flist.end(); it++)
    {
      double em_cost = it->emission_costs[0] + it->emission_costs[1] + it->emission_costs[2] + it->emission_costs[3];
      if (em_cost < best_em_cost)
        best_em_cost = em_cost;
    }

    cost += best_em_cost;// GlobalVars::lmServer->GetContextFreeCost(formID);


    word_cost_vec.push_back(pair<u16string, double>(sim_word, cost));

  }

  std::sort(word_cost_vec.begin(), word_cost_vec.end(), pair_u16string_double_comparer() );
  //std::sort(word_cost_vec.begin(), word_cost_vec.end(), [](const pair<u16string, double> &a, const pair<u16string, double> &b) { return a.second < b.second; } );

  vector<string> ret;
  for (uint32_t i = 0; i < word_cost_vec.size(); i++)
  {
    ret.push_back(UTF::UTF16To8(word_cost_vec[i].first));
  }

  return ret;

}


void Spellchecker::FindMisspelledWord(const string &text, uint32_t &range_from, uint32_t &range_length)
{
  u16string u_text = UTF::UTF8To16(text);

  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);

  for (auto it = tokens.begin(); it != tokens.end(); it++)
  {
    for (auto it2 = it->begin(); it2 != it->end(); it2++)
    {
      if ((*it2)->isUnknown())
      {
        range_from = (*it2)->first;
        range_length = (*it2)->length;

        return;
//        SimWordsFinderP swfs = configuration->simWordsFinder SimWordsFinderSpelling::CreateWithCustomMaxCosts(GlobalVars::errorModelTypoP, ErrorModelP(), true, 0, 0);
//
//        //TODO: check if word is needed here!
//        Token tok = Token((*it2)->str);
//        Similar_Words_Map swm = swfs->Find(tok, true);
//
//        if (swm.empty() || swm.begin()->second.second > 0)
//        {
//          range_from = (*it2)->first;
//          range_length = (*it2)->length;
//          return;
//        }
      }
    }
  }

  range_from = 0;
  range_length = 0;
}

vector<TextCheckingResultP> Spellchecker::GetCheckingResultsFirstSentence(const string &text, unsigned &range_from, unsigned &range_length)
{
  u16string u_text = UTF::UTF8To16(text);
  vector<TextCheckingResultP> results;

  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);

  if (tokens.empty() || tokens[0].empty())
  {
    range_from = 0;
    range_length = 0;
    return results;
  }

  string first_sentence_end = tokens[0].back()->str_utf8;
  if (first_sentence_end != ":" && first_sentence_end != "." && first_sentence_end != "?" && first_sentence_end != ".")
  {
    range_from = 0;
    range_length = 0;
    return results;
  }

  range_from = tokens[0][0]->first;
  range_length = tokens[0].back()->first + tokens[0].back()->length;

  results = GetCheckingResults(text.substr(range_from, range_length));

  return results;
}



vector<TextCheckingResultP> Spellchecker::GetCheckingResults(const string &text)
{
  u16string u_text = UTF::UTF8To16(text);
  vector<TextCheckingResultP> results;

  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);


  for (auto it = tokens.begin(); it != tokens.end(); it++)
  {
    vector<StagePossibilityP> spv;
    StagePossibilitiesType stage_posibs;
    decoder->DecodeTokenizedSentence_ReturnStagePossibilities(*it, spv, stage_posibs);
    map<uint32_t, vector<StagePossibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

    uint32_t decoder_order = decoder->GetViterbiOrder();

    for (auto it2 = sugg.begin(); it2 != sugg.end(); it2++)
    {
      //assert(it2->first >= decoder_order - 1);
      uint32_t tok_index = it2->first - (decoder_order - 1);
      vector<StagePossibilityP> &stage_pos = it2->second;


      assert(it2->first >= decoder_order - 1);
      TokenP curr_token = (*it)[tok_index];
      vector<string> suggestions;

      for (uint32_t i = 0; i < stage_pos.size(); i++)
      {
        suggestions.push_back(stage_pos[i]->ToString());
      }

      if (suggestions.size() > 0 &&
          !UTF::EqualIgnoringCase(stage_pos[0]->To_u16string(), curr_token->str_u16))
      {
        string sugg = suggestions[0];
        suggestions.clear();
        suggestions.push_back(sugg);
        results.push_back(TextCheckingResultP(new GrammarCheckingResult(curr_token->first, curr_token->length, suggestions, "Error in capitalization? Did you mean " + suggestions[0] + "?", curr_token->str_utf8)));
      }
      else if (curr_token->ID == (int)Constants::name_id ||
               curr_token->ID == (int)Constants::unknown_word_id || curr_token->ID < 0)
      {
        results.push_back(TextCheckingResultP(new SpellingCheckingResult(curr_token->first, curr_token->length, suggestions, curr_token->str_utf8)));
      }
      else if (suggestions.size() > 0) //It's not intuitive that size can be zero! However it's possible with agressive prunning during the decoding!
      {
        results.push_back(TextCheckingResultP(new GrammarCheckingResult(curr_token->first, curr_token->length, suggestions, "Did you mean " + suggestions[0] + "?", curr_token->str_utf8)));
      }
    }

  }

  return results;
}

/// @brief returns autocorrected text
///
///
string Spellchecker::CheckText(const string &text)
{
  u16string u_text = UTF::UTF8To16(text);
  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);

  u16stringReplacing usr = u16stringReplacing(u_text);

  for (auto it = tokens.begin(); it != tokens.end(); it++)
  {
    vector<StagePossibilityP> stage_pos = decoder->DecodeTokenizedSentence(*it);

    for (unsigned i = 0; i < (*it).size(); i++)
    {
      if (stage_pos[i + decoder->GetViterbiOrder() - 1]->IsOriginal() == false)
      {

        TokenP token = (*it)[i];
        StagePossibilityP st_pos = stage_pos[i + decoder->GetViterbiOrder() - 1];
        usr.Replace(token->first, token->length, st_pos->To_u16string());
      }
    }

  }

  return UTF::UTF16To8(usr.GetResult());
}

string Spellchecker::DecodeEvaluation(const string &text, uint32_t num_sugg_to_output)
{
  stringstream ret;
  u16string u_text = UTF::UTF8To16(text);
  uint32_t decoder_order = decoder->GetViterbiOrder();

  vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

  vector<StagePossibilityP> spv;
  StagePossibilitiesType stage_posibs;

  for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
  {
    vector<TokenP> &tokens = *it;
    decoder->DecodeTokenizedSentence_ReturnStagePossibilities(tokens, spv, stage_posibs);

    map<uint32_t, vector<StagePossibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);


    for (uint32_t i = 0; i < tokens.size(); i++)
    {
      if (i > 0) ret << " ";

      auto fit = sugg.find(i + decoder_order - 1); //!!!!!! i + decoder_order - 1

      if (fit == sugg.end())
      {
        if (spv[i + decoder_order - 1]->IsUnknown()) //!!!i + decoder_order - 1
        {
          //cerr << "spv->IsUnknown: " << tokens[i].GetWordStringISO() << endl;
          ret << tokens[i]->str_utf8 << "(spelling[])";
        }
        else
          ret << tokens[i]->str_utf8;
      }
      else
      {
        ret << tokens[i]->str_utf8;

        if (tokens[i]->isUnknown())
          ret << "(spelling[";
        else
          ret << "(grammar[";

        vector<StagePossibilityP> vec_sp = fit->second;

        //assert(vec_sp.size() > 0);

        for (uint32_t g = 0; g < min(vec_sp.size(), (size_t)num_sugg_to_output); g++)
        {
          if (g > 0) ret << "|";
          ret << vec_sp[g]->ToString();
        }

        ret << "])";
      }

    }
  }


  return ret.str();
}


string Spellchecker::command_line_mode(const string &text, uint32_t num_sugg_to_output)
{
  stringstream ret;

  u16string u_text = UTF::UTF8To16(text);
  uint32_t decoder_order = decoder->GetViterbiOrder();

  vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

  vector<StagePossibilityP> spv;
  StagePossibilitiesType stage_posibs;

  u16stringReplacing usr = u16stringReplacing(u_text);

  for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
  {
    vector<TokenP> &tokens = *it;
    if (tokens.empty())
      continue;

    decoder->DecodeTokenizedSentence_ReturnStagePossibilities(tokens, spv, stage_posibs);

    map<uint32_t, vector<StagePossibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);


    for (uint32_t i = 0; i < tokens.size(); i++)
    {
      auto fit = sugg.find(i + decoder_order - 1); //!!! i + decoder_order - 1

      if (fit == sugg.end())
      {
        if (spv[i + decoder_order - 1]->IsUnknown()) //!!! i + decoder_order - 1
        {
          u16string us = UTF::UTF8To16("<spelling original=\"");
          us += tokens[i]->str_u16;
          us += UTF::UTF8To16("\" suggestion=\"\"/>");
          //cerr << "spv->IsUnknown: " << tokens[i].GetWordStringISO() << endl;
          usr.Replace(tokens[i]->first, tokens[i]->length, us);
        }

      }
      else
      {
        u16string us;
        if (tokens[i]->isUnknown())
          us += UTF::UTF8To16("<spelling original=\"");
        else
          us.append(UTF::UTF8To16("<grammar original=\""));

        us.append(tokens[i]->str_u16);

        us.append(UTF::UTF8To16("\" suggestions=\""));

        vector<StagePossibilityP> vec_sp = fit->second;

        //assert(vec_sp.size() > 0);

        for (uint32_t g = 0; g < min(vec_sp.size(), (size_t)num_sugg_to_output); g++)
        {
          if (g > 0) us.append(UTF::UTF8To16("|"));
          us.append(vec_sp[g]->To_u16string());
        }

        us.append(UTF::UTF8To16("\"/>"));

        usr.Replace(tokens[i]->first, tokens[i]->length, us);
      }

    }

  }

  return UTF::UTF16To8(usr.GetResult());
}

void Spellchecker::GetSuggestions(const string &text, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions) {
  suggestions.clear();

  u16string u_text = UTF::UTF8To16(text);
  vector<vector<TokenP> > sentences = configuration->tokenizer->Tokenize(u_text);

  unsigned u_index = 0;
  for (auto&& sentence : sentences) {
    if (sentence.empty()) continue;

    vector<StagePossibilityP> spv;
    StagePossibilitiesType stage_posibs;
    decoder->DecodeTokenizedSentence_ReturnStagePossibilities(sentence, spv, stage_posibs);
    map<uint32_t, vector<StagePossibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

    uint32_t decoder_order = decoder->GetViterbiOrder();
    for (auto&& suggestion : sugg) {
      uint32_t tok_index = suggestion.first - (decoder_order - 1);
      if (tok_index >= sentence.size()) continue;

      if (u_index < sentence[tok_index]->first)
        suggestions.emplace_back(UTF::UTF16To8(u_text.substr(u_index, sentence[tok_index]->first - u_index)), vector<string>());

      suggestions.emplace_back(sentence[tok_index]->str_utf8, vector<string>());
      for (unsigned i = 0; i < suggestion.second.size() && i < num_sugg_to_output; i++)
        suggestions.back().second.emplace_back(suggestion.second[i]->ToString());

      u_index = sentence[tok_index]->first + sentence[tok_index]->length;
    }
  }
  if (u_index < u_text.size()) suggestions.emplace_back(UTF::UTF16To8(u_text.substr(u_index)), vector<string>());
}

void Spellchecker::GetTokenizedSuggestions(const vector<TokenP>& tokens, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions) {
  suggestions.clear();

  vector<StagePossibilityP> spv;
  StagePossibilitiesType stage_posibs;
  decoder->DecodeTokenizedSentence_ReturnStagePossibilities(tokens, spv, stage_posibs);
  map<uint32_t, vector<StagePossibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

  for (unsigned i = 0; i < tokens.size(); i++) {
    auto sugg_it = sugg.find(i + decoder->GetViterbiOrder() - 1);

    suggestions.emplace_back(tokens[i]->str_utf8, vector<string>());
    if (sugg_it != sugg.end())
      for (unsigned j = 0; j < sugg_it->second.size() && j < num_sugg_to_output; j++)
        suggestions.back().second.emplace_back(sugg_it->second[j]->ToString());
  }
}

void Spellchecker::Spellcheck(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned alternatives) {
  // Run Viterbi
  vector<StagePossibilityP> decoded_corrections;
  StagePossibilitiesType decoded_alternatives;
  decoder->DecodeTokenizedSentence_ReturnStagePossibilities(tokens, decoded_corrections, decoded_alternatives);

  // Data structures for alternatives selection
  struct AlternativeWithCost {
    double cost;
    StagePossibility* alternative; // Not const as no StagePossibility methods are marked const

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
    if (correction->IsOriginal()) {
      corrections.emplace_back(SpellcheckerCorrection::NONE);
    } else {
      corrections.emplace_back(tokens[i]->isUnknown() ? SpellcheckerCorrection::SPELLING : SpellcheckerCorrection::GRAMMAR);
      corrections.back().correction = correction->To_u16string();

      // Add alternatives if requested
      if (alternatives) {
        alternatives_cost.reserve(8 + 2 * decoded_alternatives->size());

        // Measure costs of alternative forms
        for (auto&& alternative : decoded_alternatives->at(i + viterbi_order - 1)) {
          if (alternative->FormIdentifier() == correction->FormIdentifier()) continue;
          auto same_alternative = alternatives_cost.find(alternative->FormIdentifier());
          bool have_current_best = same_alternative != alternatives_cost.end() || alternatives_cost.size() == alternatives;
          double current_best = same_alternative != alternatives_cost.end() ? same_alternative->second.cost :
              !alternatives_heap.empty() ? alternatives_heap.top().cost : 0;

          // Measure emmision probability
          double cost = alternative->EmmisionProbability();
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
              if (alternatives_heap.top().alternative->FormIdentifier() == alternative->FormIdentifier()) {
                alternatives_heap.pop();
                while (alternatives_cost.at(alternatives_heap.top().alternative->FormIdentifier()) < alternatives_heap.top())
                  alternatives_heap.pop();
              }
            }
          } else {
            if (alternatives_cost.size() < alternatives || cost < current_best) {
              if (alternatives_cost.size() >= alternatives) {
                alternatives_cost.erase(alternatives_heap.top().alternative->FormIdentifier());
                alternatives_heap.pop();
                // Normalize heap, i.e. remove dead top elements
                while (alternatives_cost.at(alternatives_heap.top().alternative->FormIdentifier()) < alternatives_heap.top())
                  alternatives_heap.pop();
              }
              alternatives_cost.emplace(alternative->FormIdentifier(), AlternativeWithCost(cost, alternative.get()));
              alternatives_heap.emplace(cost, alternative.get());
            }
          }
        }
        decoded_corrections[i + viterbi_order - 1] = correction;

        // Store best alternatives
        corrections.back().alternatives.resize(alternatives_cost.size());
        for (unsigned i = alternatives_cost.size(); i > 0; i--) {
          corrections.back().alternatives[i - 1] = alternatives_heap.top().alternative->To_u16string();
          alternatives_cost.erase(alternatives_heap.top().alternative->FormIdentifier());
          alternatives_heap.pop();
          if (i > 1)
            // Normalize heap, i.e. remove dead top elements
            while (!alternatives_heap.empty() && alternatives_cost.at(alternatives_heap.top().alternative->FormIdentifier())< alternatives_heap.top())
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
    if (corrections.top().correction != token->str_u16) {
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
