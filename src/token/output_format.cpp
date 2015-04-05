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

void OutputFormat::SetBlock(const string& /*block*/) {}

void OutputFormat::FinishBlock(string& /*output*/) {}

class HorizontalOutputFormat : public OutputFormat {
 public:
  virtual bool CanHandleAlternatives() const override {
    return false;
  }

  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) override {
    assert(corrections.size() >= tokens.size());

    for (unsigned i = 0; i < tokens.size(); i++) {
      if (i) output += ' ';
      UTF::UTF16To8Append(corrections[i].type == SpellcheckerCorrection::NONE ? tokens[i]->str : corrections[i].correction, output);
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
      UTF::UTF16To8Append(tokens[i]->str, output);
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

class OriginalOutputFormat : public OutputFormat {
 public:
  virtual bool CanHandleAlternatives() const override {
    return false;
  }

  virtual void SetBlock(const string& block) override {
    this->block.clear();
    UTF::UTF8To16Append(block, this->block);
    unprinted = 0;
  }

  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) override {
    assert(corrections.size() >= tokens.size());

    for (unsigned i = 0; i < tokens.size(); i++) {
      if (unprinted < tokens[i]->first) UTF::UTF16To8Append(block, unprinted, tokens[i]->first - unprinted, output);
      UTF::UTF16To8Append(corrections[i].type == SpellcheckerCorrection::NONE ? tokens[i]->str : corrections[i].correction, output);
      unprinted = tokens[i]->first + tokens[i]->length;
    }
  }

  virtual void FinishBlock(string& output) {
    if (unprinted < block.size()) {
      UTF::UTF16To8Append(block, unprinted, block.size() - unprinted, output);
      unprinted = block.size();
    }
  }

 private:
  u16string block;
  unsigned unprinted = 0;
};

unique_ptr<OutputFormat> OutputFormat::NewOriginalOutputFormat() {
  return unique_ptr<OutputFormat>(new OriginalOutputFormat());
}

class XmlOutputFormat : public OutputFormat {
 public:
  virtual bool CanHandleAlternatives() const override {
    return true;
  }

  virtual void SetBlock(const string& block) override {
    this->block.clear();
    UTF::UTF8To16Append(block, this->block);
    unprinted = 0;
  }

  virtual void AppendSentence(string& output, const vector<TokenP>& tokens, const vector<SpellcheckerCorrection>& corrections) override {
    assert(corrections.size() >= tokens.size());

    for (unsigned i = 0; i < tokens.size(); i++) {
      if (unprinted < tokens[i]->first) UTF::UTF16To8Append(block, unprinted, tokens[i]->first - unprinted, output);
      if (corrections[i].type != SpellcheckerCorrection::NONE) {
        output.append(corrections[i].type == SpellcheckerCorrection::SPELLING ? "<spelling suggestions=\"" : "<grammar suggestions=\"");
        UTF16To8AppendXmlEncoded(corrections[i].correction, output);
        for (auto&& alternative : corrections[i].alternatives) {
          output.push_back(' ');
          UTF16To8AppendXmlEncoded(alternative, output);
        }
        output.append("\">");
      }
      UTF16To8AppendXmlEncoded(tokens[i]->str, output);
      if (corrections[i].type != SpellcheckerCorrection::NONE)
        output.append(corrections[i].type == SpellcheckerCorrection::SPELLING ? "</spelling>" : "</grammar>");
      unprinted = tokens[i]->first + tokens[i]->length;
    }
  }

  virtual void FinishBlock(string& output) {
    if (unprinted < block.size()) {
      UTF::UTF16To8Append(block, unprinted, block.size() - unprinted, output);
      unprinted = block.size();
    }
  }

 private:
  void UTF16To8AppendXmlEncoded(const u16string& data, string& output) {
    encoded.clear();
    for (auto&& chr : data)
      switch(chr) {
        case '<': encoded.push_back('&'); encoded.push_back('l'); encoded.push_back('t'); encoded.push_back(';'); break;
        case '>': encoded.push_back('&'); encoded.push_back('g'); encoded.push_back('t'); encoded.push_back(';'); break;
        case '&': encoded.push_back('&'); encoded.push_back('a'); encoded.push_back('m'); encoded.push_back('p'); encoded.push_back(';'); break;
        case '"': encoded.push_back('&'); encoded.push_back('q'); encoded.push_back('u'); encoded.push_back('o'); encoded.push_back('t'); encoded.push_back(';'); break;
        default: encoded.push_back(chr);
      }

    UTF::UTF16To8Append(encoded, output);
  }

  u16string block;
  u16string encoded;
  unsigned unprinted = 0;
};

unique_ptr<OutputFormat> OutputFormat::NewXmlOutputFormat() {
  return unique_ptr<OutputFormat>(new XmlOutputFormat());
}

unique_ptr<OutputFormat> OutputFormat::NewOutputFormat(const string& name) {
  if (name == "horizontal") return NewHorizontalOutputFormat();
  if (name == "vertical") return NewVerticalOutputFormat();
  if (name == "original") return NewOriginalOutputFormat();
  if (name == "xml") return NewXmlOutputFormat();
  return nullptr;
}

} // namespace korektor
} // namespace ufal
