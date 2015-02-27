// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <map>

#include "common.h"
#include "decoder/stage_possibility.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

class DecoderBase;
SP_DEF(DecoderBase);

class TextCheckingResult;
SP_DEF(TextCheckingResult);

class Token;
SP_DEF(Token);

//Spellchecker class the main interface of the application. It receives spell-checking request and delivers the corrected text.
//The responsibility of Spellchecker is to prepare input for the decoder (i.e. tokenize the sentence), call decoder and process the decoder output in the desired way.
class Spellchecker {

  Configuration* configuration;

  DecoderBaseP decoder;

  map<uint32_t, vector<StagePossibilityP> > MakeSuggestionList(vector<StagePossibilityP> &decoded_pos, StagePossibilitiesType stage_posibilities);

 public:

  /// @brief this was relevant for the spell-server front-end
  vector<TextCheckingResultP> GetCheckingResults(const string &sentence);

  /// @brief this was relevant for the spell-server front-end
  vector<string> GetContextFreeSuggestions(const string &word);

  /// @brief this was relevant for the spell-server front-end
  void FindMisspelledWord(const string &text, uint32_t &range_from, uint32_t &range_length);

  /// @brief this was relevant for the spell-server front-end
  vector<TextCheckingResultP> GetCheckingResultsFirstSentence(const string &text, unsigned &range_from, unsigned &range_length);

  /// @brief returns autocorrected text
  string CheckText(const string &sentence);

  /// @brief marks correction in a way that is useful for evaluation - make distinction between real-word-errors and normal spelling errors
  string DecodeEvaluation(const string &text, uint32_t num_sugg_to_output);

  /// @brief marks mispelled words and correction using a XML tag
  string command_line_mode(const string &text, uint32_t num_sugg_to_output);

  /// @brief return array of tokens, for each token list suggestions from the most probable
  void GetSuggestions(const string &text, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions);
  void GetTokenizedSuggestions(const vector<TokenP>& tokens, uint32_t num_sugg_to_output, vector<pair<string, vector<string>>>& suggestions);

  Spellchecker(Configuration* _configuration);

};

} // namespace korektor
} // namespace ufal
