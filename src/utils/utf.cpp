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

using namespace unilib;

bool UTF::IsPunct(char16_t ch16)
{
  return unicode::category(ch16) & unicode::P;
}

bool UTF::IsAlpha(char16_t ch16)
{
  return unicode::category(ch16) & unicode::L;
}

bool UTF::IsAlphaNum(char16_t ch16)
{
  return unicode::category(ch16) & (unicode::L | unicode::Nd);
}

bool UTF::IsUpper(char16_t ch16)
{
  return ch16 != ToLower(ch16);
}

bool UTF::IsLower(char16_t ch16)
{
  return ch16 != ToUpper(ch16);
}

char16_t UTF::ToLower(char16_t ch16)
{
  char32_t result = unicode::lowercase(ch16);
  if (char16_t(result) == result) return result;
  return ch16;
}

char16_t UTF::ToUpper(char16_t ch16)
{
  char32_t result = unicode::uppercase(ch16);
  if (char16_t(result) == result) return result;
  return ch16;
}

string UTF::UTF16To8(const u16string &utf16)
{
  string result;
  result.reserve(2 * utf16.size());

  const char16_t* input = utf16.c_str();
  for (char32_t chr32; (chr32 = utf16::decode(input)); )
    utf8::append(result, chr32);

  return result;
}

void UTF::UTF16To8Append(const u16string &utf16, string& text) {
  text.reserve(text.size() + 2 * utf16.size());

  const char16_t* input = utf16.c_str();
  for (char32_t chr32; (chr32 = utf16::decode(input)); )
    utf8::append(text, chr32);
}

void UTF::UTF16To8Append(const u16string &utf16, size_t index, size_t length, string& text) {
  if (index >= utf16.size()) return;

  text.reserve(text.size() + 2 * length);

  const char16_t* input = utf16.c_str() + index;
  for (char32_t chr32; (chr32 = utf16::decode(input)) && length; length--)
    utf8::append(text, chr32);
}

u16string UTF::UTF8To16(const string &utf8)
{
  u16string result;
  result.reserve(utf8.size());

  const char* input = utf8.c_str();
  for (char32_t chr32; (chr32 = utf8::decode(input)); )
    utf16::append(result, chr32);

  return result;
}

void UTF::UTF8To16Append(const string &utf8, u16string& text) {
  text.reserve(text.size() + utf8.size());

  const char* input = utf8.c_str();
  for (char32_t chr32; (chr32 = utf8::decode(input)); )
    utf16::append(text, chr32);
}

bool UTF::EqualIgnoringCase(const u16string &str1, const u16string &str2)
{
  if (str1.length() != str2.length())
    return false;

  for (unsigned i = 0; i < str1.length(); i++)
  {
    if (ToLower(str1[i]) != ToLower(str1[i]))
      return false;
  }

  return true;
}

bool UTF::ContainsLetter(const u16string &ustr)
{
  for (auto&& uchar : ustr)
    if (IsAlpha(uchar))
      return true;

  return false;
}


} // namespace korektor
} // namespace ufal
