/// @file token.h
/// @brief Definition of a token
/// @date 2012
/// @copyright
/// Copyright (c) 2012, Charles University in Prague
/// All rights reserved.
///
///
///

/*
   Copyright (c) 2012, Charles University in Prague
   All rights reserved.
   */

#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include "common.h"

namespace ngramchecker {

struct Token {
  uint first;
  uint length;
  bool initialized;
  bool correction_is_allowed;
  int ID;
  u16string str_u16;
  string str_utf8;
  bool sentence_start;

  Token(uint _first, uint _length, const u16string &_str);

  void InitLexiconInformation(uint _ID, bool _correction_is_allowed);
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

}

#endif /* TOKEN_HPP_ */
