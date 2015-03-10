// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "output_format.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

class HorizontalOutputFormat : public OutputFormat {
 public:
  virtual bool CanHandleAlternatives() const override {
    return false;
  }

  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) override {
    assert(corrections.size() >= tokens.size());

    for (unsigned i = 0; i < tokens.size(); i++) {
      if (i) output += ' ';
      UTF::UTF16To8Append(corrections[i].type == SpellcheckerCorrection::NONE ? tokens[i]->str_u16 : corrections[i].correction, output);
    }
    output += '\n';
  }
};

unique_ptr<OutputFormat> OutputFormat::NewHorizontalOutputFormat() {
  return unique_ptr<OutputFormat>(new HorizontalOutputFormat());
}

class VerticalOutputFormat : public OutputFormat {
 public:
  virtual bool CanHandleAlternatives() const override {
    return true;
  }

  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) override {
    assert(corrections.size() >= tokens.size());

    for (unsigned i = 0; i < tokens.size(); i++) {
      UTF::UTF16To8Append(tokens[i]->str_u16, output);
      if (corrections[i].type != SpellcheckerCorrection::NONE) {
        output += '\t';
        output += corrections[i].type == SpellcheckerCorrection::SPELLING ? 'S' : 'G';
        output += '\t';
        UTF::UTF16To8Append(corrections[i].correction, output);
        for (auto&& alternative : corrections[i].alternatives) {
          output += '\t';
          UTF::UTF16To8Append(alternative, output);
        }
      }
      output += '\n';
    }
    output += '\n';
  }
};

unique_ptr<OutputFormat> OutputFormat::NewVerticalOutputFormat() {
  return unique_ptr<OutputFormat>(new VerticalOutputFormat());
}

unique_ptr<OutputFormat> OutputFormat::NewOutputFormat(const string& name) {
  if (name == "horizontal") return NewHorizontalOutputFormat();
  if (name == "vertical") return NewVerticalOutputFormat();
  return nullptr;
}

} // namespace korektor
} // namespace ufal
