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
  UntokenizedInputFormat(LexiconP lexicon, bool segment_on_newline, bool segment_on_punctuation)
      : segment_on_newline(segment_on_newline), segment_on_punctuation(segment_on_punctuation) {
    tokenizer.initLexicon(lexicon);
  }

  virtual bool ReadBlock(istream& ifs, string& block) override {
    block.clear();
    while (IO::ReadLine(ifs, line)) {
      block.append(line);
      block.push_back('\n');
      if (segment_on_newline || line.empty()) break;
    }
    return !block.empty();
  }

  virtual void SetBlock(const string& block) override {
    sentences = tokenizer.Tokenize(UTF::UTF8To16(block), segment_on_newline, segment_on_punctuation);
    sentence = 0;
  }

  virtual bool NextSentence(vector<TokenP>& tokens) override {
    if (sentence >= sentences.size()) return false;
    tokens = sentences[sentence++];
    return true;
  }

 private:
  bool segment_on_newline, segment_on_punctuation;

  Tokenizer tokenizer;
  string line;
  vector<vector<TokenP>> sentences;
  unsigned sentence = 0;
};

unique_ptr<InputFormat> InputFormat::NewUntokenizedInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new UntokenizedInputFormat(lexicon, false, true));
}

unique_ptr<InputFormat> InputFormat::NewUntokenizedLinesInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new UntokenizedInputFormat(lexicon, true, true));
}

unique_ptr<InputFormat> InputFormat::NewSegmentedInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new UntokenizedInputFormat(lexicon, true, false));
}

class VerticalInputFormat : public InputFormat {
 public:
  VerticalInputFormat(LexiconP lexicon) : lexicon(lexicon) {}

  virtual bool ReadBlock(istream& ifs, string& block) override {
    block.clear();
    while (IO::ReadLine(ifs, line)) {
      block.append(line);
      block.push_back('\n');
      if (line.empty()) break;
    }
    return !block.empty();
  }

  virtual void SetBlock(const string& block) override {
    index = 0;
    text.clear();
    UTF::UTF8To16Append(block, text);
  }

  virtual bool NextSentence(vector<TokenP>& tokens) override {
    tokens.clear();
    while (index < text.size()) {
      // Find next newline
      auto start = index;
      while (index < text.size() && text[index] != '\n') index++;

      // Stop on empty line if we have non-empty sentence
      if (index == start) {
        index++; // Skip over newline
        if (tokens.empty()) continue;
        break;
      }

      // Append new token
      tokens.emplace_back(new Token(start, index - start, text.substr(start, index - start)));
      tokens.back()->sentence_start = tokens.size() == 1;
      if (lexicon) {
        int id = lexicon->GetWordID(tokens.back()->str);
        tokens.back()->InitLexiconInformation(id, lexicon->CorrectionIsAllowed(id));
      }
      index++; // Skip over newline
    }

    return !tokens.empty();
  }

 private:
  LexiconP lexicon;
  string line;
  u16string text;
  size_t index = 0;
};

unique_ptr<InputFormat> InputFormat::NewVerticalInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new VerticalInputFormat(lexicon));
}

class HorizontalInputFormat : public InputFormat {
 public:
  HorizontalInputFormat(LexiconP lexicon) : lexicon(lexicon) {}

  virtual bool ReadBlock(istream& ifs, string& block) override {
    if (!IO::ReadLine(ifs, block)) return false;
    block.push_back('\n');
    return true;
  }

  virtual void SetBlock(const string& block) override {
    index = 0;
    text.clear();
    UTF::UTF8To16Append(block, text);
  }

  virtual bool NextSentence(vector<TokenP>& tokens) override {
    tokens.clear();
    while (index < text.size()) {
      // Stop on a newline unless the sentence is empty
      if (text[index] == '\n') {
        index++; // Skip over newline
        if (tokens.empty()) continue;
        break;
      }

      // Ignore spaces
      if (UTF::IsSpace(text[index])) {
        index++; // Skip over space
        continue;
      }

      // Append new token
      auto start = index;
      while (index < text.size() && !UTF::IsSpace(text[index])) index++;
      tokens.emplace_back(new Token(start, index - start, text.substr(start, index - start)));
      tokens.back()->sentence_start = tokens.size() == 1;
      if (lexicon) {
        int id = lexicon->GetWordID(tokens.back()->str);
        tokens.back()->InitLexiconInformation(id, lexicon->CorrectionIsAllowed(id));
      }
    }

    return !tokens.empty();
  }

 private:
  LexiconP lexicon;
  u16string text;
  size_t index = 0;
};

unique_ptr<InputFormat> InputFormat::NewHorizontalInputFormat(LexiconP lexicon) {
  return unique_ptr<InputFormat>(new HorizontalInputFormat(lexicon));
}

unique_ptr<InputFormat> InputFormat::NewInputFormat(const string& name, LexiconP lexicon) {
  if (name == "untokenized") return NewUntokenizedInputFormat(lexicon);
  if (name == "untokenized_lines") return NewUntokenizedLinesInputFormat(lexicon);
  if (name == "segmented") return NewSegmentedInputFormat(lexicon);
  if (name == "vertical") return NewVerticalInputFormat(lexicon);
  if (name == "horizontal") return NewHorizontalInputFormat(lexicon);
  return nullptr;
}

} // namespace korektor
} // namespace ufal
