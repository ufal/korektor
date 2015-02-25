/// @file configuration.h

#ifndef _KOREKTOR_CONFIGURATION_HPP
#define _KOREKTOR_CONFIGURATION_HPP

namespace ngramchecker {

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

class StagePosibility;
SP_DEF(StagePosibility);

class SimWordsFinder;
SP_DEF(SimWordsFinder);

struct string_bool_hasher : std::unary_function<pair<string, bool>, size_t>
{
  std::hash<string> hasher;
  size_t operator()(const pair<string, bool> &val) const
  {
    return hasher(val.first);
  }
};


/// @class Configuration configuration.h "configuration.h"
class Configuration
{
  map<string, uint> factor_map;
  vector<string> factor_names;
  vector<bool> enabled_factors;
  vector<float> factor_weights;
  vector<uint> factor_orders;
  vector<LMWrapperP> factor_LMS;
  uint last_enabled_factor_index;

  string ConvertPathSeparators(const string &path);

 public:

  uint viterbi_order;
  LexiconP lexicon;
  MorphologyP morphology;
  TokenizerP tokenizer;
  ErrorModelP errorModel;
  SimWordsFinderP simWordsFinder;
  string mode_string;
  bool diagnostics;

  Configuration() {}

  /// @brief Initialization from configuration file
  /// @param conf_file filename
  Configuration(const string &conf_file);

  bool is_initialized()
  {
    return viterbi_order > 0 && viterbi_order < 50 && lexicon && morphology
        && tokenizer && errorModel && simWordsFinder;
  }

  inline LMWrapper* GetFactorLM(uint index) { return factor_LMS[index].get(); }
  inline float GetFactorWeight(uint index) { return factor_weights[index]; }
  inline uint GetFactorOrder(uint index) { return factor_orders[index]; }
  inline bool IsFactorEnabled(uint index) { return enabled_factors[index]; }
  inline uint NumFactors() { return factor_LMS.size(); }
  inline uint GetLastEnabledFactorIndex() { return last_enabled_factor_index; }

  inline uint ViterbiOrder()
  {
    return viterbi_order;
  }

  inline bool FactorIsEnabled(uint index)
  {
    return enabled_factors[index];
  }

  /// @brief Loads lexicon and morphology
  void LoadMorphologyAndLexicon(LexiconP _lexicon, MorphologyP _morphology);

  /// @brief Loads the language model
  void LoadLM(LMWrapperP lm);

  void EnableFactor(const string &fac_name, float weight, uint order);

};

SP_DEF(Configuration);
}

#endif //_KOREKTOR_CONFIGURATION_HPP
