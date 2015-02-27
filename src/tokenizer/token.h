// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file token.h
/// @brief Definition of a token

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

struct Token {
  unsigned first;
  unsigned length;
  bool initialized;
  bool correction_is_allowed;
  int ID;
  u16string str_u16;
  string str_utf8;
  bool sentence_start;

  Token(unsigned _first, unsigned _length, const u16string &_str);

  void InitLexiconInformation(unsigned _ID, bool _correction_is_allowed);
  inline bool isUnknown() { return ID == -1; }

  Token(const u16string &u_str);
};

SP_DEF(Token);

struct TokenP_compare: std::less<TokenP>
{
  bool operator()(TokenP const& x, TokenP const& y) const
  {
    if (x->first < y->first)
      return true;
    else
      return false;
  }
};

} // namespace korektor
} // namespace ufal
