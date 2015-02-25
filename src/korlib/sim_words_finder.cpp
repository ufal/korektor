// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "sim_words_finder.h"
#include "token.h"
#include "lexicon.h"
#include "error_model.h"
#include "configuration.h"
#include "utils.h"

namespace ngramchecker {

void SimWordsFinder::Find_basic(const TokenP &token, uint32_t lookup_max_ed_dist, double lookup_max_cost, Similar_Words_Map &ret)
{
  u16string &word_u_str = token->str_u16;

  if (token->correction_is_allowed == false)
  {

    u16stringP word_strP = u16stringP(new u16string(word_u_str));

    ret[token->ID] = make_pair(word_strP, 0.0);
  }
  else
  {

    ret = configuration->lexicon->GetSimilarWords(word_u_str, lookup_max_ed_dist, lookup_max_cost, configuration->errorModel, false);

    if (token->sentence_start && MyUtils::IsUpperCase(word_u_str[0])) //On the beginning of the sentence both lower case and upper case starting letter should be tried!
    {
      // Make sure all suggestions are capitalized.
      for (auto&& suggestion : ret) {
        auto& word = *suggestion.second.first;
        if (!word.empty() && MyUtils::IsLowerCase(word[0]))
          word[0] = MyUtils::ToUpper(word[0]);
      }

      // Try searching for word without first capital letter.
      u16string word_str_lc = word_u_str;
      word_str_lc[0] = MyUtils::ToLower(word_u_str[0]);

      Similar_Words_Map msw_lc = configuration->lexicon->GetSimilarWords(word_str_lc, lookup_max_ed_dist, lookup_max_cost, configuration->errorModel, false);

      for (Similar_Words_Map::iterator it = msw_lc.begin(); it != msw_lc.end(); it++)
      {
        //word form of founded suggestion should be capitalized!
        u16string &ustring = *(it->second.first);
        if (MyUtils::IsLowerCase(ustring[0]))
          ustring[0] = MyUtils::ToUpper(ustring[0]);

        Similar_Words_Map::iterator fit = ret.find(it->first);
        if (fit == ret.end())
        {
          ret[it->first] = it->second;
        }
        else
        {
          if (it->second.second < ret[it->first].second)
          {
            ret[it->first].second = it->second.second;
            ret[it->first].first = it->second.first;
          }
        }
      }
    }

  }

}

void SimWordsFinder::Find_basic_ignore_case(const TokenP &token, bool keep_orig_casing, uint32_t lookup_max_ed_dist, double lookup_max_cost, Similar_Words_Map &ret)
{
  u16string &word_u_str = token->str_u16;
  ret = configuration->lexicon->GetSimilarWords(word_u_str, lookup_max_ed_dist, lookup_max_cost, configuration->errorModel, true);

  capitalization_type ct = MyUtils::Get_u16string_capitalization_type(word_u_str);

  if (keep_orig_casing == true)
  {
    for (Similar_Words_Map::iterator it = ret.begin(); it != ret.end(); it++)
    {

      u16string &sim_w_str = *(it->second.first);

      for (uint32_t j = 0; j < sim_w_str.length(); j++)
      {
        switch (ct)
        {
          case all_upper_case: sim_w_str[j] = MyUtils::ToUpper(sim_w_str[j]); break;
          case all_lower_case: sim_w_str[j] = MyUtils::ToLower(sim_w_str[j]); break;
          case first_upper_case: if ( j == 0)
                                   sim_w_str[j] = MyUtils::ToUpper(sim_w_str[j]);
                                 else
                                   sim_w_str[j] = MyUtils::ToLower(sim_w_str[j]);
                                 break;
          case weird: break;
        }
      }

    }

  }
  else if (token->sentence_start && MyUtils::IsUpperCase(word_u_str[0]))
  {
    for (Similar_Words_Map::iterator it = ret.begin(); it != ret.end(); it++)
    {

      u16string &sim_w_str = *(it->second.first);

      if (MyUtils::IsLowerCase(sim_w_str[0]))
      {
        sim_w_str[0] = MyUtils::ToUpper(sim_w_str[0]);
      }

    }

  }
}

Similar_Words_Map SimWordsFinder::Find(const TokenP &token)
{
  Similar_Words_Map swm;

  uint i = 0;
  while (swm.empty() && i < search_configs.size())
  {
    SearchConfig sc = search_configs[i];

    if (sc.casing == case_sensitive)
    {
      Find_basic(token, sc.max_ed_dist, sc.max_cost, swm);
    }
    else if (sc.casing == ignore_case)
    {
      Find_basic_ignore_case(token, false, sc.max_ed_dist, sc.max_cost, swm);
    }
    else
    {
      Find_basic_ignore_case(token, true, sc.max_ed_dist, sc.max_cost, swm);
    }

    i++;
  }

  return swm;
}

}
