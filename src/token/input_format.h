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
#include "token.h"

namespace ufal {
namespace korektor {

class Lexicon;
SP_DEF(Lexicon);

class InputFormat {
 public:
  virtual ~InputFormat() {}

  virtual bool ReadBlock(istream& ifs, string& block) = 0;

  virtual void SetBlock(const string& block) = 0;
  virtual bool NextSentence(vector<TokenP>& tokens) = 0;

  static unique_ptr<InputFormat> NewInputFormat(const string& name, LexiconP lexicon);
  static unique_ptr<InputFormat> NewUntokenizedInputFormat(LexiconP lexicon);
};

} // namespace korektor
} // namespace ufal
