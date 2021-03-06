// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file error_model_basic.h
/// @class ErrorModelBasic error_model.h "error_model.h"
/// @brief This class implements the error model
///
/// @todo <OL>
///   <LI>Replace the general purpose hash table with custom linear hashing container (since there will be no removing of elements) - it will improve memory use patterns, i.e. greater locality etc. </LI>
/// </OL>

#pragma once

#include <unordered_map>
#include <tuple>

#include "common.h"
#include "error_model.h"
#include "utils/hash.h"

namespace ufal {
namespace korektor {

class ErrorModelBasic;
SP_DEF(ErrorModelBasic);

class ErrorModelBasic : public ErrorModel
{
 private:
  struct char16_pair_hash
  {
    size_t operator()(const pair<char16_t, char16_t> &a) const
    {
      size_t seed = 0;
      Hash::Combine(seed, a.first);
      Hash::Combine(seed, a.second);
      return seed;
    }
  };
  struct char16_triple_hash
  {
    size_t operator()(const tuple<char16_t, char16_t, char16_t> &a) const
    {
      size_t seed = 0;
      Hash::Combine(seed, get<0>(a));
      Hash::Combine(seed, get<1>(a));
      Hash::Combine(seed, get<2>(a));
      return seed;
    }
  };

  unordered_map<pair<char16_t, char16_t>, ErrorModelOutput, char16_pair_hash> substitution_map; ///< Substitution map
  unordered_map<tuple<char16_t, char16_t, char16_t>, ErrorModelOutput, char16_triple_hash> insertion_map; ///< Insertion map
  unordered_map<pair<char16_t, char16_t>, ErrorModelOutput, char16_pair_hash> deletion_map; ///< Deletion map
  unordered_map<pair<char16_t, char16_t>, ErrorModelOutput, char16_pair_hash> swap_map; ///< Swap map

  //if the operation cost is not found in the particular hash map, then the default cost and edit distance is used
  ErrorModelOutput substitution_default; ///< Default substitution cost/edit distance
  ErrorModelOutput insertion_default; ///< Default insertion cost/edit distance
  ErrorModelOutput deletion_default; ///< Default deletion cost/edit distance
  ErrorModelOutput swap_default; ///< Default swap cost/edit distance
  ErrorModelOutput case_mismatch_cost; ///< Default case mismatch cost/edit distance
  uint32_t min_operation_edit_distance; ///< @todo variable for ?

 public:

  /// @brief Constructor initialization through parameters
  ///
  /// @param _min_operation_edit_distance Minimum operation edit distance
  /// @param _case_mismatch_cost Cost for characters that differ only by case
  /// @param _substitution_default Default substitution cost/distance
  /// @param _insertion_default Default insertion cost/distance
  /// @param _deletion_default Default deletion cost/distance
  /// @param _swap_default Default swap cost/distance
  /// @param entries Error corpus. Vector of pair of strings and costs/distances
  ErrorModelBasic(unsigned _min_operation_edit_distance, ErrorModelOutput _case_mismatch_cost,
                  ErrorModelOutput _substitution_default, ErrorModelOutput _insertion_default,
                  ErrorModelOutput _deletion_default, ErrorModelOutput _swap_default,
                  const vector<pair<u16string, ErrorModelOutput>> &entries);

  /// @brief Write the error model object to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs);

  /// @brief Create binary output from text input
  ///
  /// @param text_input Text input
  /// @param binary_output Binary output
  static void CreateBinaryFormFromTextForm(const string &text_input, const string &binary_output);

  /// @brief Constructor initialization from input stream.
  ///
  /// @param ifs Input stream
  ErrorModelBasic(istream &ifs);

  /// @brief Initialize the @ref ErrorModelBasic object from binary file
  ///
  /// @param binary_file File name
  /// @return Pointer ( @ref ErrorModelBasic ) to error model object
  static ErrorModelBasicP fromBinaryFile(string binary_file);


  /// @brief Calculate substitution cost
  ///
  /// @param char1 A character
  /// @param char2 A character
  /// @param ignore_case "true" or "false", whether to ignore the case difference
  /// @return An object of type @ref ErrorModelOutput containing the substitution cost and edit distance
  virtual ErrorModelOutput SubstitutionCost(char16_t char1, char16_t char2, bool ignore_case = false);

  /// @brief Calculate insertion cost
  ///
  /// @param inserted_char Inserted character
  /// @param previous_char Previous character
  /// @param next_char Next character
  /// @return An object of type @ref ErrorModelOutput containing the insertion cost and edit distance
  virtual ErrorModelOutput InsertionCost(char16_t inserted_char, char16_t previous_char, char16_t next_char);

  /// @brief Calculate swap cost
  ///
  /// @param first_char A character
  /// @param second_char A character
  /// @return An object of type @ref ErrorModelOutput containing the swap cost and edit distance
  virtual ErrorModelOutput SwapCost(char16_t first_char, char16_t second_char);

  /// @brief Calculate deletion cost
  ///
  /// @param current_char Current character
  /// @param previous_char Previous character
  /// @return An object of type @ref ErrorModelOutput containing the deletion cost and edit distance
  virtual ErrorModelOutput DeletionCost(char16_t current_char, char16_t previous_char);

  /// @todo Function for ?
  virtual uint32_t MinOperationEditDistance() { return min_operation_edit_distance; }

  /// @brief Check whether two strings are identical
  ///
  /// @param s1 A string
  /// @param s2 A string
  /// @return Boolean
  virtual bool StringsAreIdentical(const u16string &s1, const u16string &s2) { return s1 == s2; }

  virtual string ToString() { return "ErrorModelBasic"; }
};

} // namespace korektor
} // namespace ufal
