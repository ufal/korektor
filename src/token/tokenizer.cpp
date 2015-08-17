// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "lexicon/lexicon.h"
#include "token.h"
#include "tokenizer.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

vector<vector<TokenP> > Tokenizer::Tokenize(const u16string &text, bool segment_on_newline, bool segment_on_punctuation)
{
  vector<TokenP> tokens;

  tokens.reserve(text.length() / 4);

  vector<unsigned> sentence_ends;

  for (unsigned i = 0; i < text.length(); i++)
  {
    if (text[i] == '\n' && (segment_on_newline || (i && text[i-1] == '\n')))
    {
      sentence_ends.push_back(i);
    }
    else if (segment_on_punctuation && (text[i] == '.' || text[i] == ':' || text[i] == '?' || text[i] == '!' || text[i] == u'\u2026'/*triple dot*/))
    {
      unsigned closing_punct = i;
      while (closing_punct + 1 < text.size() && UTF::IsPunctClosing(text[closing_punct+1]))
        closing_punct++;

      unsigned first_letter = closing_punct;
      while (first_letter + 1 < text.size() && (UTF::IsSpace(text[first_letter+1]) || UTF::IsPunctOpening(text[first_letter+1])))
        first_letter++;
      if (first_letter + 1 >= text.size() || UTF::IsUpper(text[first_letter+1]))
          sentence_ends.push_back(closing_punct);
    }
  }

  unsigned i = 0;

  while (i < text.length())
  {
    if (UTF::IsAlphaNum(text[i]))
    {
      unsigned length = 1;
      while (i + length < text.length() && UTF::IsAlphaNum(text[i + length]))
        length++;

      TokenP token = TokenP(new Token(i, length, text.substr(i, length)));

      if (lexicon)
      {
        int wordID = lexicon->GetWordID(token->str);
        token->InitLexiconInformation(wordID, lexicon->CorrectionIsAllowed(wordID));
      }

      tokens.push_back(token);

      i += length;
    }
    else if (!UTF::IsSpace(text[i]))
    {
      TokenP token = TokenP(new Token(i, 1, text.substr(i, 1)));

      if (lexicon)
      {
        int wordID = lexicon->GetWordID(token->str);
        token->InitLexiconInformation(wordID, lexicon->CorrectionIsAllowed(wordID));
      }

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
  bool sentence_start = true;

  for (unsigned i = 0; i < tokens.size(); i++)
  {
    while (next_sentence_end_index < sentence_ends.size() && tokens[i]->first > sentence_ends[next_sentence_end_index])
    {
      next_sentence_end_index++;
      sentence_start = true;
    }

    tokens[i]->sentence_start = sentence_start;
    if (sentence_start) {
      ret.emplace_back();
      sentence_start = false;
    }
    ret.back().push_back(tokens[i]);
  }

  return ret;
}

} // namespace korektor
} // namespace ufal
