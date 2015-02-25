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

#include "spellchecker.h"
#include "text_checking_result.h"
#include "token.h"
#include "tokenizer.h"
//#include "stage_posibility.h"
#include "decoder_base.h"
#include "decoder_multi_factor.h"
#include "configuration.h"
#include "u16string_replacing.h"
#include "sim_words_finder.h"
#include "morphology.h"
#include "constants.h"

namespace ngramchecker {

map<uint32_t, vector<StagePosibilityP> > Spellchecker::MakeSuggestionList(vector<StagePosibilityP> &decoded_pos, StagePosibilitiesType stage_posibilities) {


  //cerr << "Make suggestion started..." << endl;
  uint32_t viterbi_order = decoder->GetViterbiOrder();

  assert(stage_posibilities->size() == decoded_pos.size());


  map<uint32_t, vector<StagePosibilityP> > suggestions;


  for (uint32_t i = viterbi_order - 1; i < decoded_pos.size() - 1; i++)
  {
    //cerr << "i = " << i << endl;

    if (decoded_pos[i]->IsOriginal())
    {
      if (decoded_pos[i]->IsUnknown())
      {
        suggestions[i] = vector<StagePosibilityP>();
      }

      continue;
    }


    //assert (i < stage_posibilities->size());
    //assert (stage_posibilities != NULL);
    vector<StagePosibilityP> st_pos = stage_posibilities->operator [](i);

    vector<pair<StagePosibilityP, double> > cost_vector;

    suggestions[i] = vector<StagePosibilityP>();

    StagePosibilityP decoded_pos_at_i_backup = decoded_pos[i];

    for (uint32_t j = 0; j < st_pos.size(); j++)
    {
      //cerr << "j = " << j << endl;
      StagePosibilityP sp = st_pos[j];
      double cost = sp->EmmisionProbability();

      decoded_pos[i] = sp;

      for (uint32_t k = 0; k < viterbi_order; k++)
      {
        if (i + k >= decoded_pos.size() )
          break;

        cost += decoder->ComputeTransitionCostSPSequence(decoded_pos, i - viterbi_order + 1 + k, i + k);
      }

      cost_vector.push_back(pair<StagePosibilityP, double>(sp, cost));

    }

    decoded_pos[i] = decoded_pos_at_i_backup;

    std::sort(cost_vector.begin(), cost_vector.end(), Pair_StagePosibilityP_double_comparer());
    vector<pair<StagePosibilityP, double> > new_cost_vector;
    set<StagePosibilityP, StagePosibility_Form_comparer> word_ids_in_list;

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


    for (vector<pair<StagePosibilityP, double> >::iterator it = new_cost_vector.begin(); it != new_cost_vector.end(); it++)
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
  u16string u_word = MyUtils::utf8_to_utf16(word);

  TokenP token(new Token(u_word));
  token->correction_is_allowed = true;
  token->sentence_start = false;

  Similar_Words_Map swm = configuration->simWordsFinder->Find(token);

  vector<pair<u16string, double> > word_cost_vec;

  for (Similar_Words_Map::iterator it = swm.begin(); it != swm.end(); it++)
  {

    uint32_t formID = it->first;
    double cost = it->second.second;
    u16string sim_word = *(it->second.first);

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
    ret.push_back(MyUtils::utf16_to_utf8(word_cost_vec[i].first));
  }

  return ret;

}


void Spellchecker::FindMisspelledWord(const string &text, uint32_t &range_from, uint32_t &range_length)
{
  u16string u_text = MyUtils::utf8_to_utf16(text);

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
        /*SimWordsFinderP swfs = configuration->simWordsFinder SimWordsFinderSpelling::CreateWithCustomMaxCosts(GlobalVars::errorModelTypoP, ErrorModelP(), true, 0, 0);

        //TODO: check if word is needed here!
        Token tok = Token((*it2)->str);
        Similar_Words_Map swm = swfs->Find(tok, true);

        if (swm.empty() || swm.begin()->second.second > 0)
        {
        range_from = (*it2)->first;
        range_length = (*it2)->length;
        return;
        }*/

      }
    }
  }

  range_from = 0;
  range_length = 0;
}

vector<TextCheckingResultP> Spellchecker::GetCheckingResultsFirstSentence(const string &text, uint &range_from, uint &range_length)
{
  u16string u_text = MyUtils::utf8_to_utf16(text);
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
  u16string u_text = MyUtils::utf8_to_utf16(text);
  vector<TextCheckingResultP> results;

  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);


  for (auto it = tokens.begin(); it != tokens.end(); it++)
  {
    vector<StagePosibilityP> spv;
    StagePosibilitiesType stage_posibs;
    decoder->DecodeTokenizedSentence_ReturnStagePosibilities(*it, spv, stage_posibs);
    map<uint32_t, vector<StagePosibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

    uint32_t decoder_order = decoder->GetViterbiOrder();

    for (map<uint32_t, vector<StagePosibilityP> >::iterator it2 = sugg.begin(); it2 != sugg.end(); it2++)
    {
      //assert(it2->first >= decoder_order - 1);
      uint32_t tok_index = it2->first - (decoder_order - 1);
      vector<StagePosibilityP> &stage_pos = it2->second;


      assert(it2->first >= decoder_order - 1);
      TokenP curr_token = (*it)[tok_index];
      vector<string> suggestions;

      for (uint32_t i = 0; i < stage_pos.size(); i++)
      {
        suggestions.push_back(stage_pos[i]->ToString());
      }

      if (suggestions.size() > 0 &&
          MyUtils::CaseInsensitiveIsEqual(stage_pos[0]->To_u16string(), curr_token->str_u16) == false)
      {
        string sugg = suggestions[0];
        suggestions.clear();
        suggestions.push_back(sugg);
        results.push_back(TextCheckingResultP(new GrammarCheckingResult(curr_token->first, curr_token->length, suggestions, "Error in capitalization? Did you mean " + suggestions[0] + "?", curr_token->str_utf8)));
      }
      else if (curr_token->ID == (int)MyConstants::name_id ||
               curr_token->ID == (int)MyConstants::unknown_word_id || curr_token->ID < 0)
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
  u16string u_text = MyUtils::utf8_to_utf16(text);
  vector<vector<TokenP> > tokens = configuration->tokenizer->Tokenize(u_text);

  u16stringReplacing usr = u16stringReplacing(u_text);

  for (auto it = tokens.begin(); it != tokens.end(); it++)
  {
    vector<StagePosibilityP> stage_pos = decoder->DecodeTokenizedSentence(*it);

    for (uint i = 0; i < (*it).size(); i++)
    {
      if (stage_pos[i + decoder->GetViterbiOrder() - 1]->IsOriginal() == false)
      {

        TokenP token = (*it)[i];
        StagePosibilityP st_pos = stage_pos[i + decoder->GetViterbiOrder() - 1];
        usr.Replace(token->first, token->length, st_pos->To_u16string());
      }
    }

  }

  return MyUtils::utf16_to_utf8(usr.GetResult());
}

string Spellchecker::DecodeEvaluation(const string &text, uint32_t num_sugg_to_output)
{
  stringstream ret;
  u16string u_text = MyUtils::utf8_to_utf16(text);
  uint32_t decoder_order = decoder->GetViterbiOrder();

  vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

  vector<StagePosibilityP> spv;
  StagePosibilitiesType stage_posibs;

  for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
  {
    vector<TokenP> &tokens = *it;
    decoder->DecodeTokenizedSentence_ReturnStagePosibilities(tokens, spv, stage_posibs);

    map<uint32_t, vector<StagePosibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);


    for (uint32_t i = 0; i < tokens.size(); i++)
    {
      if (i > 0) ret << " ";

      map<uint32_t, vector<StagePosibilityP> >::iterator fit = sugg.find(i + decoder_order - 1); //!!!!!! i + decoder_order - 1

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

        vector<StagePosibilityP> vec_sp = fit->second;

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

  u16string u_text = MyUtils::utf8_to_utf16(text);
  uint32_t decoder_order = decoder->GetViterbiOrder();

  vector<vector<TokenP> > tokens_all = configuration->tokenizer->Tokenize(u_text);

  vector<StagePosibilityP> spv;
  StagePosibilitiesType stage_posibs;

  u16stringReplacing usr = u16stringReplacing(u_text);

  for (auto it = tokens_all.begin(); it != tokens_all.end(); it++)
  {
    vector<TokenP> &tokens = *it;
    if (tokens.empty())
      continue;

    decoder->DecodeTokenizedSentence_ReturnStagePosibilities(tokens, spv, stage_posibs);

    map<uint32_t, vector<StagePosibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);


    for (uint32_t i = 0; i < tokens.size(); i++)
    {
      map<uint32_t, vector<StagePosibilityP> >::iterator fit = sugg.find(i + decoder_order - 1); //!!! i + decoder_order - 1

      if (fit == sugg.end())
      {
        if (spv[i + decoder_order - 1]->IsUnknown()) //!!! i + decoder_order - 1
        {
          u16string us = MyUtils::utf8_to_utf16("<spelling original=\"");
          us += tokens[i]->str_u16;
          us += MyUtils::utf8_to_utf16("\" suggestion=\"\"/>");
          //cerr << "spv->IsUnknown: " << tokens[i].GetWordStringISO() << endl;
          usr.Replace(tokens[i]->first, tokens[i]->length, us);
        }

      }
      else
      {
        u16string us;
        if (tokens[i]->isUnknown())
          us += MyUtils::utf8_to_utf16("<spelling original=\"");
        else
          us.append(MyUtils::utf8_to_utf16("<grammar original=\""));

        us.append(tokens[i]->str_u16);

        us.append(MyUtils::utf8_to_utf16("\" suggestions=\""));

        vector<StagePosibilityP> vec_sp = fit->second;

        //assert(vec_sp.size() > 0);

        for (uint32_t g = 0; g < min(vec_sp.size(), (size_t)num_sugg_to_output); g++)
        {
          if (g > 0) us.append(MyUtils::utf8_to_utf16("|"));
          us.append(vec_sp[g]->To_u16string());
        }

        us.append(MyUtils::utf8_to_utf16("\"/>"));

        usr.Replace(tokens[i]->first, tokens[i]->length, us);
      }

    }

  }

  return MyUtils::utf16_to_utf8(usr.GetResult());
}

void Spellchecker::GetSuggestions(const string &text, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions) {
  suggestions.clear();

  u16string u_text = MyUtils::utf8_to_utf16(text);
  vector<vector<TokenP> > sentences = configuration->tokenizer->Tokenize(u_text);

  unsigned u_index = 0;
  for (auto&& sentence : sentences) {
    if (sentence.empty()) continue;

    vector<StagePosibilityP> spv;
    StagePosibilitiesType stage_posibs;
    decoder->DecodeTokenizedSentence_ReturnStagePosibilities(sentence, spv, stage_posibs);
    map<uint32_t, vector<StagePosibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

    uint32_t decoder_order = decoder->GetViterbiOrder();
    for (auto&& suggestion : sugg) {
      uint32_t tok_index = suggestion.first - (decoder_order - 1);
      if (tok_index >= sentence.size()) continue;

      if (u_index < sentence[tok_index]->first)
        suggestions.emplace_back(MyUtils::utf16_to_utf8(u_text.substr(u_index, sentence[tok_index]->first - u_index)), vector<string>());

      suggestions.emplace_back(sentence[tok_index]->str_utf8, vector<string>());
      for (unsigned i = 0; i < suggestion.second.size() && i < num_sugg_to_output; i++)
        suggestions.back().second.emplace_back(suggestion.second[i]->ToString());

      u_index = sentence[tok_index]->first + sentence[tok_index]->length;
    }
  }
  if (u_index < u_text.size()) suggestions.emplace_back(MyUtils::utf16_to_utf8(u_text.substr(u_index)), vector<string>());
}

void Spellchecker::GetTokenizedSuggestions(const vector<TokenP>& tokens, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions) {
  suggestions.clear();

  vector<StagePosibilityP> spv;
  StagePosibilitiesType stage_posibs;
  decoder->DecodeTokenizedSentence_ReturnStagePosibilities(tokens, spv, stage_posibs);
  map<uint32_t, vector<StagePosibilityP> > sugg = MakeSuggestionList(spv, stage_posibs);

  for (unsigned i = 0; i < tokens.size(); i++) {
    auto sugg_it = sugg.find(i + decoder->GetViterbiOrder() - 1);

    suggestions.emplace_back(tokens[i]->str_utf8, vector<string>());
    if (sugg_it != sugg.end())
      for (unsigned j = 0; j < sugg_it->second.size() && j < num_sugg_to_output; j++)
        suggestions.back().second.emplace_back(sugg_it->second[j]->ToString());
  }
}


Spellchecker::Spellchecker(Configuration* _configuration):
  configuration(_configuration), decoder(new DecoderMultiFactor(_configuration))
{}

}

