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

Token::Token(unsigned _first, unsigned _length, const u16string& str) :
  first(_first), length(_length), initialized(false), correction_is_allowed(false), ID(-1),
  str(str), sentence_start(false)
{}

Token::Token(const u16string& str) :
  first(0), length(0), initialized(false), correction_is_allowed(false), ID(-1),
  str(str), sentence_start(false)
{}

void Token::InitLexiconInformation(unsigned _ID, bool _correction_is_allowed)
{
  initialized = true;
  ID = _ID;
  correction_is_allowed = UTF::ContainsLetter(str) && _correction_is_allowed;
}

} // namespace korektor
} // namespace ufal
