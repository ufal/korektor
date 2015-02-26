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
#include "lexicon.h"
#include "token.h"
#include "utils.h"


namespace ufal {
namespace korektor {

enum matcher_type { undef, sentence_end, special_class, token };

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
  vector<vector<TokenP> > Tokenize(const u16string &text)
  {
    vector<TokenP> tokens;

    tokens.reserve(text.length() / 4);

    vector<unsigned> sentence_ends;

    for (unsigned i = 0; i < text.length(); i++)
    {
      if (text[i] == '\n' || text[i] == ':' || text[i] == '?' || text[i] == '!') sentence_ends.push_back(i);
      else if (text[i] == '.')
      {
        unsigned j = i + 1;
        while (j < text.length() - 1 && MyUTF::is_alphanum(text[j]) == false) j++;

        if (MyUtils::IsUpperCase(text[j]))
          sentence_ends.push_back(i);
      }
    }

    unsigned i = 0;

    while (i < text.length())
    {
      if (MyUTF::is_alphanum(text[i]))
      {
        unsigned length = 1;
        while (i + length < text.length() && MyUTF::is_alphanum(text[i + length]))
          length++;

        TokenP token = TokenP(new Token(i, length, text.substr(i, length)));

        if (lexicon)
        {
          int wordID = lexicon->GetWordID(token->str_u16);
          token->InitLexiconInformation(wordID, lexicon->CorrectionIsAllowed(wordID));
        }
        token->correction_is_allowed = token->correction_is_allowed && MyUtils::ContainsLetter(token->str_u16);

        tokens.push_back(token);

        i += length;
      }
      else if (MyUTF::is_punct(text[i]))
      {
        TokenP token = TokenP(new Token(i, 1, text.substr(i, 1)));

        if (lexicon)
        {
          int wordID = lexicon->GetWordID(token->str_u16);
          token->InitLexiconInformation(wordID, lexicon->CorrectionIsAllowed(wordID));
        }
        token->correction_is_allowed = token->correction_is_allowed && MyUtils::ContainsLetter(token->str_u16);

        tokens.push_back(token);

        i++;
      }
      else
      {
        i++;
      }
    }

    unsigned next_sentence_end_index = 0;

    vector<vector<TokenP> > ret;
    ret.push_back(vector<TokenP>());

    for (unsigned i = 0; i < tokens.size(); i++)
    {
      while (next_sentence_end_index < sentence_ends.size() && tokens[i]->first > sentence_ends[next_sentence_end_index])
      {
        next_sentence_end_index++;
        ret.push_back(vector<TokenP>());
      }

      if (ret.back().empty())
        tokens[i]->sentence_start = true;
      else
        tokens[i]->sentence_start = false;
      ret.back().push_back(tokens[i]);
    }

    return ret;

  }
};

SP_DEF(Tokenizer);

} // namespace korektor
} // namespace ufal
