// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "utf.h"

#include "unilib/utf8.h"
#include "unilib/utf16.h"
#include "unilib/unicode.h"

namespace ufal {
namespace korektor {

char16_t UTF::tolower(char16_t ch16)
{
  char32_t result = unicode::lowercase(ch16);
  if (char16_t(result) == result) return result;
  return ch16;
}

char16_t UTF::toupper(char16_t ch16)
{
  char32_t result = unicode::uppercase(ch16);
  if (char16_t(result) == result) return result;
  return ch16;
}

string UTF::utf16_to_utf8(const u16string &utf16)
{
  u32string utf32;
  utf16::decode(utf16, utf32);

  string result;
  utf8::encode(utf32, result);

  return result;
}

u16string UTF::utf8_to_utf16(const string &utf8)
{
  u32string utf32;
  utf8::decode(utf8, utf32);

  u16string result;
  utf16::encode(utf32, result);

  return result;
}

bool UTF::is_punct(char16_t ch16)
{
  return unicode::category(ch16) & unicode::P;
}

bool UTF::is_alphanum(char16_t ch16)
{
  return unicode::category(ch16) & (unicode::L | unicode::Nd);
}

bool UTF::is_alpha(char16_t ch16)
{
  return unicode::category(ch16) & unicode::L;
}

} // namespace korektor
} // namespace ufal
