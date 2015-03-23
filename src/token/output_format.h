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
#include "spellchecker/spellchecker_correction.h"
#include "token.h"

namespace ufal {
namespace korektor {

class OutputFormat {
 public:
  virtual ~OutputFormat() {}

  virtual bool CanHandleAlternatives() const = 0;

  virtual void SetBlock(const string& /*block*/) {}
  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) = 0;
  virtual void FinishBlock(string& /*output*/) {}

  static unique_ptr<OutputFormat> NewOutputFormat(const string& name);
  static unique_ptr<OutputFormat> NewHorizontalOutputFormat();
  static unique_ptr<OutputFormat> NewVerticalOutputFormat();
  static unique_ptr<OutputFormat> NewOriginalOutputFormat();
  static unique_ptr<OutputFormat> NewXmlOutputFormat();
};

} // namespace korektor
} // namespace ufal
