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
#include "spellchecker_correction.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

class DecoderBase;
SP_DEF(DecoderBase);

struct Token;
SP_DEF(Token);

//Spellchecker class the main interface of the application. It receives spell-checking request and delivers the corrected text.
//The responsibility of Spellchecker is to prepare input for the decoder (i.e. tokenize the sentence), call decoder and process the decoder output in the desired way.
class Spellchecker {

  Configuration* configuration;

  DecoderBaseP decoder;

 public:
  Spellchecker(Configuration* _configuration);

  void Spellcheck(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned alternatives = 0);
  void SpellcheckToken(const TokenP& token, SpellcheckerCorrection& correction, unsigned alternatives = 0);
};

} // namespace korektor
} // namespace ufal
