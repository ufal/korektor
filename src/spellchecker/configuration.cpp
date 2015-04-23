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

#include "configuration.h"
#include "constants.h"
#include "error_model/error_model_basic.h"
#include "language_model/lm_wrapper.h"
#include "language_model/zip_lm.h"
#include "lexicon/lexicon.h"
#include "lexicon/sim_words_finder.h"
#include "morphology/morphology.h"
#include "token/tokenizer.h"
#include "utils/io.h"
#include "utils/parse.h"

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
  ifs.open(conf_file.c_str());

  if (ifs.is_open() == false)
    runtime_failure("Opening configuration file '" << conf_file << "' failed!");

  model_order = 1;
  viterbi_order = 0;
  viterbi_beam_size = 0;
  viterbi_stage_pruning = Constants::prunning_constant;

  vector<SimWordsFinder::SearchConfig> search_configs;

  string s;

  while (IO::ReadLine(ifs, s))
  {
    if (s == "" || s[0] == '#')  //comment
      continue;

    if (s.substr(0, 9) == "morpholex")
    {
      string morpholex_file = configuration_directory + ConvertPathSeparators(s.substr(10));
      ifstream ifs;
      ifs.open(morpholex_file.c_str(), ios::binary);
      if (ifs.is_open() == false)
        runtime_failure("Opening morpholex file '" << morpholex_file << "' failed!");

      MorphologyP morphology = MorphologyP(new Morphology(ifs));
      LexiconP lexicon = LexiconP(new Lexicon(ifs));

      LoadMorphologyAndLexicon(lexicon, morphology);

      ifs.close();
    }
    else if (s.substr(0, 10) == "errormodel")
    {
      string error_model_file = configuration_directory + ConvertPathSeparators(s.substr(11));

      ErrorModelBasicP emb = ErrorModelBasic::fromBinaryFile(error_model_file);
      errorModel = emb;
    }
    else if (s.substr(0, 2) == "lm")
    {
      vector<string> toks;

      IO::Split(s, '-', toks);

      if (toks.size() != 4)
        runtime_failure("Not four hyphen-separated columns on line '" << s << "' in file '" << conf_file << "'!");

      ZipLMP lm = ZipLMP(new ZipLM(configuration_directory + ConvertPathSeparators(toks[1])));
      LMWrapperP lm_wrapper = LMWrapperP(new LMWrapper(lm));
      LoadLM(lm_wrapper);

      string order_str = toks[2];
      unsigned order = Parse::Int(order_str, "lm order");

      string weight_str = toks[3];
      float weight = Parse::Double(weight_str, "lm weight");

      EnableFactor(lm->GetFactorName(), weight, order);

      if (model_order < order)
        model_order = order;
    }
    else if (s.substr(0, 6) == "search")
    {
      vector<string> toks;
      IO::Split(s, '-', toks);

      if (toks.size() < 4)
        runtime_failure("Less than four hyphen-separated columns on line '" << s << "' in file '" << conf_file << "'!");
      if (toks.size() > 6)
        runtime_failure("More than six hyphen-separated columns on line '" << s << "' in file '" << conf_file << "'!");

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

      unsigned max_edit_distance = Parse::Int(toks[2], "search max edit distance");
      float max_cost = Parse::Double(toks[3], "search max cost");
      unsigned min_length = toks.size() > 4 ? Parse::Int(toks[4], "minimum length") : 0;
      unsigned max_length = toks.size() > 5 ? Parse::Int(toks[5], "maximum length") : 0;

      search_configs.push_back(SimWordsFinder::SearchConfig(ct, max_edit_distance, max_cost, min_length, max_length));
    }
    else if (s.substr(0, 4) == "mode")
    {
      // Mode is now ignored, but it is accepted so that old configuration files work.
    }
    else if (s.compare(0, 11, "diagnostics") == 0)
    {
      diagnostics = true;
      morphology->initMorphoWordLists(configuration_directory + ConvertPathSeparators(s.substr(12)));
      morphology->initMorphoWordMaps();
    }
    else if (s.compare(0, 13, "viterbi_order") == 0)
    {
      int order = Parse::Int(s.substr(14), "viterbi order");
      if (order <= 0) runtime_failure("Specified viterbi order '" << order << "' must be greater than zero!");
      viterbi_order = order;
    }
    else if (s.compare(0, 17, "viterbi_beam_size") == 0)
    {
      int beam_size = Parse::Int(s.substr(18), "viterbi beam size");
      if (beam_size <= 0) runtime_failure("Specified viterbi beam size '" << beam_size << "' must be greater than zero!");
      viterbi_beam_size = beam_size;
    }
    else if (s.compare(0, 21, "viterbi_stage_pruning") == 0)
    {
      double pruning = Parse::Double(s.substr(22), "viterbi stage pruning");
      if (pruning <= 0) runtime_failure("Specified viterbi stage pruning '" << pruning << "' must be greater than zero!");
      viterbi_stage_pruning = pruning;
    }
  }

  tokenizer = TokenizerP(new Tokenizer() );
  tokenizer->initLexicon(lexicon);

  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::case_sensitive, 1, 6));
  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 1, 6));
  //search_configs.push_back(SimWordsFinder::SearchConfig(SimWordsFinder::ignore_case, 2, 9));

  simWordsFinder = SimWordsFinderP(new SimWordsFinder(this, search_configs));

  if (!viterbi_order) viterbi_order = model_order - 1;

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
