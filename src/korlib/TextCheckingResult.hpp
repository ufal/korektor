/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#ifndef TEXTCHECKINGRESULT_HPP_
#define TEXTCHECKINGRESULT_HPP_

#include "StdAfx.h"

namespace ngramchecker {

enum TextCheckingResultType { grammar, spelling };

class TextCheckingResult {
 public:
  uint32_t range_from;
  uint32_t range_length;
  vector<string> suggestions;
  TextCheckingResultType type;
  string orig_word;

  TextCheckingResult(uint32_t _range_from, uint32_t _range_length, vector<string> &_suggestions, TextCheckingResultType _type, string _orig_word);

  virtual shared_ptr<TextCheckingResult> makeShiftedCopy(uint32_t shift) = 0;

  virtual string ToString() = 0;

};

class SpellingCheckingResult : public TextCheckingResult {
 public:

  SpellingCheckingResult(uint32_t _range_from, uint32_t _range_length, vector<string> &_suggestions, string _orig_word);

  virtual shared_ptr<TextCheckingResult> makeShiftedCopy(uint32_t shift);

  virtual string ToString();

};

class GrammarCheckingResult : public TextCheckingResult {
 public:
  string error_description;

  GrammarCheckingResult(uint32_t _range_from, uint32_t _range_until, vector<string> &_suggestions, string _error_description, string _orig_word);

  virtual shared_ptr<TextCheckingResult> makeShiftedCopy(uint32_t shift);

  virtual string ToString();

};

SP_DEF(GrammarCheckingResult);
SP_DEF(SpellingCheckingResult);

SP_DEF(TextCheckingResult);
typedef shared_ptr<vector<shared_ptr<TextCheckingResult> > > vectorP_TextCheckingResultP;

}

#endif /* TEXTCHECKINGRESULT_HPP_ */
