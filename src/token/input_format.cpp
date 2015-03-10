// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "input_format.h"
#include "lexicon/lexicon.h"
#include "tokenizer.h"
#include "utils/io.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

class UntokenizedInputFormat : public InputFormat {
 public:
  UntokenizedInputFormat(LexiconP lexicon, bool split_on_newline) : split_on_newline(split_on_newline) {
    tokenizer.initLexicon(lexicon);
  }

  virtual bool ReadBlock(istream& ifs, string& block) override {
    block.clear();
    while (IO::ReadLine(ifs, line)) {
      block.append(line);
      block.push_back('\n');
      if (split_on_newline || line.empty()) break;
    }
    return !block.empty();
  }

  virtual void SetBlock(const string& block) override {
    sentences = tokenizer.Tokenize(UTF::UTF8To16(block), split_on_newline);
    sentence = 0;
  }

  virtual bool NextSentence(vector<TokenP>& tokens) override {
    if (sentence >= sentences.size()) return false;
    tokens = sentences[sentence++];
    return true;
  }

 private:
  bool split_on_newline;

  Tokenizer tokenizer;
  string line;
  vector<vector<TokenP>> sentences;
  unsigned sentence = 0;
};

unique_ptr<InputFormat> InputFormat::NewUntokenizedInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new UntokenizedInputFormat(lexicon, false));
}

unique_ptr<InputFormat> InputFormat::NewUntokenizedLinesInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new UntokenizedInputFormat(lexicon, true));
}

unique_ptr<InputFormat> InputFormat::NewInputFormat(const string& name, LexiconP lexicon) {
  if (name == "untokenized") return NewUntokenizedInputFormat(lexicon);
  if (name == "untokenized_lines") return NewUntokenizedLinesInputFormat(lexicon);
  return nullptr;
}

} // namespace korektor
} // namespace ufal
