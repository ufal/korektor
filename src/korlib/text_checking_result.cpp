// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "text_checking_result.h"

namespace ufal {
namespace korektor {

TextCheckingResult::TextCheckingResult(uint32_t _range_from, uint32_t _range_length, vector<string> &_suggestions, TextCheckingResultType _type, string _orig_word):
  range_from(_range_from), range_length(_range_length), type(_type), orig_word(_orig_word)
{
  for (uint32_t i = 0; i < _suggestions.size(); i++)
  {
    suggestions.push_back(_suggestions[i]);
  }

}

SpellingCheckingResult::SpellingCheckingResult(uint32_t _range_from, uint32_t _range_length, vector<string> &_suggestions, string _orig_word):
  TextCheckingResult(_range_from, _range_length, _suggestions, spelling, _orig_word)
{}

shared_ptr<TextCheckingResult> SpellingCheckingResult::makeShiftedCopy(uint32_t shift)
{
  shared_ptr<TextCheckingResult> ret = shared_ptr<TextCheckingResult>(new SpellingCheckingResult(range_from + shift, range_length, suggestions, orig_word));
  return ret;
}

string SpellingCheckingResult::ToString()
{
  stringstream strs;
  string ret;
  strs << "{" << range_from << ", " << range_length << "}: Spelling (";

  for (uint32_t i = 0; i < suggestions.size(); i++)
    strs << " " << suggestions[i];

  strs << " )";


  ret = strs.str();
  return ret;
}

GrammarCheckingResult::GrammarCheckingResult(uint32_t _range_from, uint32_t _range_until, vector<string> &_suggestions, string _error_description, string _orig_word):
  TextCheckingResult(_range_from, _range_until, _suggestions, grammar, _orig_word), error_description(_error_description) {}

shared_ptr<TextCheckingResult> GrammarCheckingResult::makeShiftedCopy(uint32_t shift)
{
  shared_ptr<TextCheckingResult> ret = shared_ptr<TextCheckingResult>(new GrammarCheckingResult(range_from + shift, range_length, suggestions, error_description, orig_word));
  return ret;
}

string GrammarCheckingResult::ToString()
{
  stringstream strs;
  string ret;
  strs << "{" << range_from << ", " << range_length << "}: Grammar (";

  for (uint32_t i = 0; i < suggestions.size(); i++)
    strs << " " << suggestions[i];

  strs << " )";
  strs << ", error_description = " << error_description << ", orig_word = " << orig_word;
  ret = strs.str();
  return ret;
}

} // namespace korektor
} // namespace ufal
