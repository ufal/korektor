// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file configuration.h

#pragma once

#include <unordered_map>

#include "common.h"

namespace ufal {
namespace korektor {

class LMWrapper;
SP_DEF(LMWrapper);

class Morphology;
SP_DEF(Morphology);

class Lexicon;
SP_DEF(Lexicon);

class Tokenizer;
SP_DEF(Tokenizer);

class ErrorModel;
SP_DEF(ErrorModel);

class StagePossibility;
SP_DEF(StagePossibility);

class SimWordsFinder;
SP_DEF(SimWordsFinder);

/// @class Configuration configuration.h "configuration.h"
class Configuration
{
  unordered_map<string, unsigned> factor_map;
  vector<string> factor_names;
  vector<bool> enabled_factors;
  vector<float> factor_weights;
  vector<unsigned> factor_orders;
  vector<LMWrapperP> factor_LMS;
  unsigned last_enabled_factor_index;

  string ConvertPathSeparators(const string &path);

 public:

  unsigned viterbi_order;
  LexiconP lexicon;
  MorphologyP morphology;
  TokenizerP tokenizer;
  ErrorModelP errorModel;
  SimWordsFinderP simWordsFinder;
  bool diagnostics;

  Configuration() : last_enabled_factor_index(0) {}

  /// @brief Initialization from configuration file
  /// @param conf_file filename
  Configuration(const string &conf_file);

  bool is_initialized()
  {
    return viterbi_order > 0 && viterbi_order < 50 && lexicon && morphology
        && tokenizer && errorModel && simWordsFinder;
  }

  inline LMWrapper* GetFactorLM(unsigned index) { return factor_LMS[index].get(); }
  inline float GetFactorWeight(unsigned index) { return factor_weights[index]; }
  inline unsigned GetFactorOrder(unsigned index) { return factor_orders[index]; }
  inline bool IsFactorEnabled(unsigned index) { return enabled_factors[index]; }
  inline unsigned NumFactors() { return factor_LMS.size(); }
  inline unsigned GetLastEnabledFactorIndex() { return last_enabled_factor_index; }

  inline unsigned ViterbiOrder()
  {
    return viterbi_order;
  }

  inline bool FactorIsEnabled(unsigned index)
  {
    return enabled_factors[index];
  }

  /// @brief Loads lexicon and morphology
  void LoadMorphologyAndLexicon(LexiconP _lexicon, MorphologyP _morphology);

  /// @brief Loads the language model
  void LoadLM(LMWrapperP lm);

  void EnableFactor(const string &fac_name, float weight, unsigned order);

};

SP_DEF(Configuration);

} // namespace korektor
} // namespace ufal
