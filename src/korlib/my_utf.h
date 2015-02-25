// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <stdint.h>
#include <string>
#include "common.h"

using namespace std;

namespace ngramchecker {

class MyPackedArray;

class MyUTF
{

  //static uint16_t* uc_to_lc;
  //static uint16_t* lc_to_uc;
  //static MyPackedArray char_types;
  //static bool initialized;

  //static bool is_non_leading_byte(char ch);

 public:

  enum character_type { alphanum, punctuation, blank, control, something_else };

  //static void init_mapping(const string &uc_to_lc_file, const string &lc_to_uc_file, const string &char_types_file);
  //static void init_mapping(const string &directory);

  static char16_t tolower(char16_t ch16);

  static char16_t toupper(char16_t ch16);

  static string utf16_to_utf8(const u16string &utf16);

  static u16string utf8_to_utf16(const string &utf8);

  static bool is_punct(char16_t ch16);
  static bool is_alphanum(char16_t ch16);
  static bool is_alpha(char16_t ch16);
};


}
