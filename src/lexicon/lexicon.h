// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file lexicon.cpp
/// @class Lexicon lexicon.h "lexicon.h"

#pragma once

#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "common.h"
#include "persistent_structures/comp_increasing_array.h"
#include "persistent_structures/packed_array.h"
#include "similar_words_map.h"

namespace ufal {
namespace korektor {

class ErrorModel;
SP_DEF(ErrorModel);

class lexicon_node;
SP_DEF(lexicon_node);

class Lexicon {

 private:

  CompIncreasingArray  states_arcpointer;
  uint32_t* arcs_nextstate; ///< Pointer to child node IDs
  char16_t* arcs_char; ///< Pointer to child node labels

  uint32_t num_words; ///< Number of words in the lexicon
  uint32_t root_id; ///< Root ID
  uint32_t num_arcs; ///< Number of edges in the lexicon

  unordered_set<uint32_t> noncorrectable_word_ids;

  uint32_t NumStates() const;

  int ContainsSuffix(uint32_t stateID, const u16string &word, uint32_t start_char_index) const;

  bool StateIsWord(uint32_t state_ID) const;

  int SingleArc_nextstate(uint32_t stateID, char16_t character) const;

  static void create_lexicon_rec(lexicon_nodeP &node, unsigned &next_inner_node_id, const u16string &curr_word, unsigned char_index, unordered_map<u16string, unsigned> &words_map);

 public:

  ~Lexicon()
  {
    delete[] arcs_char;
    delete[] arcs_nextstate;
  }

  Lexicon& operator=(const Lexicon& val);

  Lexicon(const Lexicon& val);

  Lexicon(istream &ifs);

  Lexicon(const vector<u16string> &words);

  /// @brief Get the number of words in the lexicon
  ///
  /// @return Number of words in the lexicon
  uint32_t NumWords() { return num_words; }

  static Lexicon fromUTF8Strings(const vector<string> &words);

  //should be called just in the preparatory phase - during the lexicon creation process!!!
  void LoadListOfNoncorrectableWords(const vector<u16string> &noncorrectable_words);

  void WriteToStream(ostream &ofs) const;

  int GetWordID(const u16string &word) const; //return value -1 for out of vocabulary words!

  void PrintWords(ostream &os, uint32_t max_index = std::numeric_limits<uint32_t>::max());

  bool CorrectionIsAllowed(int wordID) const;
  void ArcsConsistencyCheck();

 private:

  void print_words_rec(unsigned node_id, u16string &prefix, vector<u16string> &words, unsigned &index, uint32_t max_index);

  void AddSimilarWordToMap(Similar_Words_Map &ret, uint32_t word_id, double cost, u16string &word, uint32_t word_include_letter_start_index, u16string &prefix) const;

  Similar_Words_Map GetSimilarWords_impl(u16string &word, uint32_t edit_distance, double cost, ErrorModelP errModel, uint32_t stateID, uint32_t startIndex, u16string &prefix, double cost_limit, bool ignore_case, uint32_t max_edit_distance);

 public:

  Similar_Words_Map GetSimilarWords(u16string word, uint32_t max_ed_dist, double _cost_limit, ErrorModelP errModel, bool _ignore_case);
  vector<u16string> GetSimilarWords_string(u16string word, uint32_t max_ed_dist, double _cost_limit, ErrorModelP errModel, bool _ignore_case);
};


SP_DEF(Lexicon);

} // namespace korektor
} // namespace ufal
