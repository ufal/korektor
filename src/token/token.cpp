// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "token.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

Token::Token(unsigned _first, unsigned _length, const u16string &_str):
  first(_first), length(_length), initialized(false), correction_is_allowed(false), str_u16(_str), str_utf8(UTF::UTF16To8(_str)) {}

Token::Token(const u16string &u_str): first(0), length(0), initialized(false),
  correction_is_allowed(false), ID(-1), str_u16(u_str), str_utf8(UTF::UTF16To8(u_str)), sentence_start(false) {}


void Token::InitLexiconInformation(unsigned _ID, bool _correction_is_allowed)
{
  initialized = true;
  ID = _ID;
  correction_is_allowed = UTF::ContainsLetter(str_u16) && _correction_is_allowed;
}

} // namespace korektor
} // namespace ufal
