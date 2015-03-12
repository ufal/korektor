// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file tokenizer.h
/// @brief Simple Tokenizer

#pragma once

#include "common.h"
#include "token.h"

namespace ufal {
namespace korektor {

class Lexicon;
SP_DEF(Lexicon);

/// @brief Tokenizer class for sentence tokenization
class Tokenizer {
  LexiconP lexicon; ///< lexicon

 public:

  Tokenizer(): lexicon(LexiconP()) {}

  void initLexicon(LexiconP _lexicon)
  {
    lexicon = _lexicon;
  }

  /// @brief Tokenize the given text
  ///
  /// This function tokenizes the given text and returns the
  /// tokens in a vector
  ///
  /// @param text The input text
  /// @return The tokens
  /// @todo The tokenization at the moment is English specific. It should be generic.
  vector<vector<TokenP> > Tokenize(const u16string &text, bool segment_on_newline = true, bool segment_on_punctuation = true);
};

SP_DEF(Tokenizer);

} // namespace korektor
} // namespace ufal
