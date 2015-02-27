// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file tokenize.cpp
/// @brief Simple tokenizer

#include <iostream>

#include "common.h"
#include "token/tokenizer.h"
#include "utils/utf.h"

using namespace ufal::korektor;

// floating point number:  [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
// date: yyyy-mm-dd        (19|20)\d\d([- /.])(0[1-9]|1[012])\2(0[1-9]|[12][0-9]|3[01])
// date: dd-mm-yyyy        (0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])[- /.](19|20)\d\d
// date: mm-dd-yyyy        (0[1-9]|[12][0-9]|3[01])[- /.](0[1-9]|1[012])[- /.](19|20)\d\d
// email:                  \b[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}\b
// url:                    \b(([\w-]+://?|www[.])[^\s()<>]+(?:\([\w\d]+\)|([^[:punct:]\s]|/)))
// number:                 \b[0-9]+\b
// sentence end:           ([.?!:]) [[:upper:]]
// token:                  \b([.<.>/?;:'"\\\\\\[{\\]}=+-_()*&^%$#@!]|\w+)\b

int main()
{
  Tokenizer tokenizer;
  string s;

  while (std::getline(cin, s))
  {
    u16string text = UTF::UTF8To16(s);

    vector<vector<TokenP> > tokens = tokenizer.Tokenize(text);
    for (auto it = tokens.begin();  it != tokens.end(); it++)
    {
      for (auto it2 = it->begin(); it2 != it->end(); it2++)
      {
        if (it2 != it->begin())
          cout << " ";
        cout << (*it2)->str_utf8;
      }
      cout << endl;
    }
  }

  return 0;
}
