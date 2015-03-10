// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class UTF
{
 public:
  static bool IsAlpha(char16_t ch16);
  static bool IsAlphaNum(char16_t ch16);
  static bool IsPunct(char16_t ch16);

  static bool IsUpper(char16_t ch16);
  static bool IsLower(char16_t ch16);

  static char16_t ToLower(char16_t ch16);
  static char16_t ToUpper(char16_t ch16);

  static string UTF16To8(const u16string &utf16);
  static void UTF16To8Append(const u16string &utf16, string& text);
  static u16string UTF8To16(const string &utf8);
  static void UTF8To16Append(const string &utf8, u16string& text);

  static bool EqualIgnoringCase(const u16string &str1, const u16string &str2);
  static bool ContainsLetter(const u16string &ustr);
};

} // namespace korektor
} // namespace ufal
