// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <algorithm>
#include <fstream>
#include <iostream>

#include "configuration.h"
#include "error_model/error_model_basic.h"
#include "language_model/lm_wrapper.h"
#include "language_model/zip_lm.h"
#include "lexicon/lexicon.h"
#include "lexicon/sim_words_finder.h"
#include "morphology/morphology.h"
#include "token/tokenizer.h"

namespace ufal {
namespace korektor {

/// @brief Loads lexicon and morphology
///
/// @param _lexicon Lexicon
/// @param _morphology Morphology
void Configuration::LoadMorphologyAndLexicon(LexiconP _lexicon, MorphologyP _morphology)
{
  lexicon = _lexicon;
  morphology = _morphology;

  factor_map = morphology->GetFactorMap();

  for (auto it = factor_map.begin(); it != factor_map.end(); it++)
    factor_names.push_back("");

  for (auto it = factor_map.begin(); it != factor_map.end(); it++)
    factor_names[it->second] = it->first;

  for (auto it = factor_names.begin(); it != factor_names.end(); it++)
  {
    enabled_factors.push_back(false);
    factor_weights.push_back(0.0f);
    factor_LMS.push_back(LMWrapperP());
    factor_orders.push_back(0);
  }

}

void Configuration::LoadLM(LMWrapperP lm)
{
  unsigned index = factor_map[lm->FactorName()];
  factor_LMS[index] = lm;
}

void Configuration::EnableFactor(const string &fac_name, float weight, unsigned order)
{
  unsigned index = factor_map[fac_name];
  enabled_factors[index] = true;
  factor_weights[index] = weight;
  factor_orders[index] = order;

  if (index > last_enabled_factor_index)
    last_enabled_factor_index = index;
}

/// @brief Initialization from configuration file
///
/// @param conf_file Absolute path to the configuration file
Configuration::Configuration(const string &conf_file)
{
  last_enabled_factor_index = 0;
  diagnostics = false;

  auto slash_pos = conf_file.find_last_of("\\/");
  string configuration_directory = slash_pos == string::npos ? string() : conf_file.substr(0, slash_pos + 1);

  ifstream ifs;
  ifs.open((conf_file).c_str());

  if (ifs.is_open() == false)
  {
    cerr << "Opening configuration file " << conf_file << "failed!" << endl;
    exit(1);
    //throw std::bad_exception("Can't open configuration file!");
  }

  viterbi_order = 1;

  vector<SimWordsFinder::SearchConfig> search_configs;

  mode_string = "tag_errors";
  string s;

  while (Utils::SafeReadline(ifs, s))
  {
    if (s == "" || s[0] == '#')  //comment
      continue;

    if (s.substr(0, 9) == "morpholex")
    {
      string morpholex_file = configuration_directory + ConvertPathSeparators(s.substr(10));
      ifstream ifs;
      ifs.open(morpholex_file.c_str(), ios::binary);
      if (ifs.is_open() == false)
      {
        cerr << "Opening morpholex file " << morpholex_file << " failed!" << endl;
        exit(1);
        //throw std::bad_exception("Can't open morpholex file!");
      }

      MorphologyP morphology = MorphologyP(new Morphology(ifs));
      LexiconP lexicon = LexiconP(new Lexicon(ifs));

      LoadMorphologyAndLexicon(lexicon, morphology);

      //cerr << "morpholex loaded!" << endl;

      ifs.close();
    }
    else if (s.substr(0, 10) == "errormodel")
    {
      string error_model_file = configuration_directory + ConvertPathSeparators(s.substr(11));

      ErrorModelBasicP emb = ErrorModelBasic::fromBinaryFile(error_model_file);
      errorModel = emb;
      //cerr << "error model loaded" << endl;
    }
    else if (s.substr(0, 2) == "lm")
    {
      vector<string> toks;

      Utils::Split(toks, s, "-");

      FATAL_CONDITION(toks.size() == 4, s);

      ZipLMP lm = ZipLMP(new ZipLM(configuration_directory + ConvertPathSeparators(toks[1])));
      LMWrapperP lm_wrapper = LMWrapperP(new LMWrapper(lm));
      LoadLM(lm_wrapper);

      string order_str = toks[2];
      unsigned order = Utils::my_atoi(order_str);

      string weight_str = toks[3];
      float weight = Utils::my_atof(weight_str);

      EnableFactor(lm->GetFactorName(), weight, order);

      if (viterbi_order < order)
        viterbi_order = order;

      //cerr << "language model " << toks[1] << " loaded!" << endl;
    }
    else if (s.substr(0, 6) == "search")
    {
      vector<string> toks;
      Utils::Split(toks, s, "-");

      FATAL_CONDITION(toks.size() == 4, s);

      SimWordsFinder::casing_treatment ct;

      if (toks[1] == "case_sensitive")
        ct = SimWordsFinder::case_sensitive;
      else if (toks[1] == "ignore_case")
        ct = SimWordsFinder::ignore_case;
      else if (toks[1] == "ignore_case_keep_orig")
        ct = SimWordsFinder::ignore_case_keep_orig;
      else
      {
        cerr << s << endl << toks[1] << " - invalid value of casing treatment" << endl;
        exit(1);
      }

      unsigned max_edit_distance = Utils::my_atoi(toks[2]);
      float max_cost = Utils::my_atof(toks[3]);

      search_configs.push_back(SimWordsFinder::SearchConfig(ct, max_edit_distance, max_cost));
    }
    else if (s.substr(0, 4) == "mode")
    {
      mode_string = s.substr(5);

      if (mode_string != "autocorrect" && mode_string != "tag_errors")
      {
        cerr << s << endl << " - invalid value of output mode" << endl;
        exit(1);
      }
    }
    else if (s.compare(0, 11, "diagnostics") == 0)
    {
      diagnostics = true;
      morphology->initMorphoWordLists(configuration_directory + ConvertPathSeparators(s.substr(12)));
      morphology->initMorphoWordMaps();
    }
  }

  tokenizer = TokenizerP(new Tokenizer() );
  tokenizer->initLexicon(lexicon);

  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::case_sensitive, 1, 6));
  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 1, 6));
  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 2, 9));

  simWordsFinder = SimWordsFinderP(new SimWordsFinder(this, search_configs));

  if (is_initialized() == false)
  {
    //throw std::bad_exception("configuration is not initialized!");
    cerr << "Configuration was not initialized proparly!" << endl;
    exit(1);
  }
}

/// @brief Convert path separators to the one used by the current OS
/// @param path Path to the file or directory
string Configuration::ConvertPathSeparators(const string &path) {
#ifdef _WIN32
  char to_replace = '/', replace_by = '\\';
#else
  char to_replace = '\\', replace_by = '/';
#endif
  string result = path;
  replace(result.begin(), result.end(), to_replace, replace_by);
  return result;
}

} // namespace korektor
} // namespace ufal
