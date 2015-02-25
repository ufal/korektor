/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#include "token.h"
#include "utils.h"

namespace ngramchecker {

Token::Token(uint _first, uint _length, const u16string &_str):
  first(_first), length(_length), initialized(false), correction_is_allowed(false), str_u16(_str), str_utf8(MyUtils::utf16_to_utf8(_str)) {}

Token::Token(const u16string &u_str): first(0), length(0), initialized(false),
  correction_is_allowed(false), ID(-1), str_u16(u_str), str_utf8(MyUtils::utf16_to_utf8(u_str)), sentence_start(false) {}


void Token::InitLexiconInformation(uint _ID, bool _correction_is_allowed)
{
  initialized = true;
  ID = _ID;
  correction_is_allowed = _correction_is_allowed;
}

}
