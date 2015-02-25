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

#ifndef _ERROR_MODEL_BASIC_HPP
#define _ERROR_MODEL_BASIC_HPP

#include <tuple>

#include "common.h"
#include "error_model.h"
#include "utils.h"

namespace ngramchecker {

class ErrorModelBasic;
SP_DEF(ErrorModelBasic);

/// @struct char16_pair_hash error_model_basic.h "error_model_basic.h"
/// @brief Generates hash value given 2 strings
struct char16_pair_hash : std::unary_function<pair<char16_t, char16_t>, size_t>
{
  size_t operator()(const pair<char16_t, char16_t> &val) const
  {
    size_t seed = 0;
    MyUtils::HashCombine(seed, val.first);
    MyUtils::HashCombine(seed, val.second);
    return seed;
  }
};
/// @struct char16_triple_hash error_model_basic.h "error_model_basic.h"
/// @brief Generates hash value given 3 strings
struct char16_triple_hash : std::unary_function<tuple<char16_t, char16_t, char16_t>, size_t>
{
  size_t operator()(const tuple<char16_t, char16_t, char16_t> &val) const
  {
    size_t seed = 0;

    MyUtils::HashCombine(seed, get<0>(val));
    MyUtils::HashCombine(seed, get<1>(val));
    MyUtils::HashCombine(seed, get<2>(val));

    return seed;
  }
};

class ErrorModelBasic : public ErrorModel
{
 private:

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
  uint min_operation_edit_distance; ///< @todo variable for ?

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
  ErrorModelBasic(uint _min_operation_edit_distance, ErrorModelOutput _case_mismatch_cost, ErrorModelOutput _substitution_default, ErrorModelOutput _insertion_default,
                  ErrorModelOutput _deletion_default, ErrorModelOutput _swap_default,
                  const vector<pair<u16string, ErrorModelOutput>> &entries):
    substitution_default(_substitution_default),
    insertion_default(_insertion_default),
    deletion_default(_deletion_default),
    swap_default(_swap_default),
    case_mismatch_cost(_case_mismatch_cost),
    min_operation_edit_distance(_min_operation_edit_distance)
  {

    for (auto it = entries.begin(); it != entries.end(); it++)
    {
      u16string signature = it->first;
      ErrorModelOutput emo = it->second;

      ErrorModelOutput emo_case_mismatch = ErrorModelOutput(emo.edit_dist + case_mismatch_cost.edit_dist, emo.cost + case_mismatch_cost.cost);

      if (signature.substr(0, 2) == MyUtils::utf8_to_utf16("s_"))
      {
        char16_t ch1 = signature[2];
        char16_t ch2 = signature[3];

        substitution_map[make_pair(ch1, ch2)] = emo;

        char16_t ch1_uc = MyUtils::ToUpper(ch1);
        char16_t ch2_uc = MyUtils::ToUpper(ch2);

        substitution_map[make_pair(ch1_uc, ch2_uc)] = emo;
        substitution_map[make_pair(ch1_uc, ch2)] = emo_case_mismatch;
        substitution_map[make_pair(ch1, ch2_uc)] = emo_case_mismatch;
      }
      else if (signature.substr(0, 5) == MyUtils::utf8_to_utf16("swap_"))
      {
        char16_t ch1 = signature[5];
        char16_t ch2 = signature[6];

        swap_map[make_pair(ch1, ch2)] = emo;

        char16_t ch1_uc = MyUtils::ToUpper(ch1);
        char16_t ch2_uc = MyUtils::ToUpper(ch2);

        swap_map[make_pair(ch1_uc, ch2_uc)] = emo;
        swap_map[make_pair(ch1_uc, ch2)] = emo;
        swap_map[make_pair(ch1, ch2_uc)] = emo;

      }
      else if (signature.substr(0, 2) == MyUtils::utf8_to_utf16("i_"))
      {
        char16_t ch1 = signature[2];
        char16_t ch2 = signature[3];
        char16_t ch3 = signature[4];

        insertion_map[make_tuple(ch1, ch2, ch3)] = emo;

        char16_t ch1_uc = MyUtils::ToUpper(ch1);
        char16_t ch2_uc = MyUtils::ToUpper(ch2);
        char16_t ch3_uc = MyUtils::ToUpper(ch3);

        insertion_map[make_tuple(ch1_uc, ch2, ch3)] = emo;
        insertion_map[make_tuple(ch1, ch2_uc, ch3)] = emo;
        insertion_map[make_tuple(ch1_uc, ch2_uc, ch3)] = emo;
        insertion_map[make_tuple(ch1, ch2, ch3_uc)] = emo;
        insertion_map[make_tuple(ch1_uc, ch2, ch3_uc)] = emo;
        insertion_map[make_tuple(ch1, ch2_uc, ch3_uc)] = emo;
        insertion_map[make_tuple(ch1_uc, ch2_uc, ch3_uc)] = emo;
      }
      else if (signature.substr(0, 2) == MyUtils::utf8_to_utf16("d_"))
      {
        char16_t ch1 = signature[2];
        char16_t ch2 = signature[3];

        deletion_map[make_pair(ch1, ch2)] = emo;

        char16_t ch1_uc = MyUtils::ToUpper(ch1);
        char16_t ch2_uc = MyUtils::ToUpper(ch2);

        deletion_map[make_pair(ch1_uc, ch2)] = emo;
        deletion_map[make_pair(ch1, ch2_uc)] = emo;
        deletion_map[make_pair(ch1_uc, ch2_uc)] = emo;
      }


    }
  }

  /// @brief Write the error model object to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs)
  {
    ofs.write((char*)&(substitution_default.cost), sizeof(float));
    ofs.write((char*)&(substitution_default.edit_dist), sizeof(uint32_t));
    ofs.write((char*)&(insertion_default.cost), sizeof(float));
    ofs.write((char*)&(insertion_default.edit_dist), sizeof(uint32_t));
    ofs.write((char*)&(deletion_default.cost), sizeof(float));
    ofs.write((char*)&(deletion_default.edit_dist), sizeof(uint32_t));
    ofs.write((char*)&(swap_default.cost), sizeof(float));
    ofs.write((char*)&(swap_default.edit_dist), sizeof(uint32_t));
    ofs.write((char*)&(case_mismatch_cost.cost), sizeof(float));
    ofs.write((char*)&(case_mismatch_cost.edit_dist), sizeof(uint32_t));
    ofs.write((char*)&min_operation_edit_distance, sizeof(uint32_t));

    uint num_subs = substitution_map.size();
    ofs.write((char*)&num_subs, sizeof(uint32_t));

    for (auto it = substitution_map.begin(); it != substitution_map.end(); it++)
    {
      ofs.write((char*)&(it->first.first), sizeof(char16_t));
      ofs.write((char*)&(it->first.second), sizeof(char16_t));
      ofs.write((char*)&(it->second.cost), sizeof(float));
      ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
    }

    uint num_inserts = insertion_map.size();
    ofs.write((char*)&num_inserts, sizeof(uint32_t));

    for (auto it = insertion_map.begin(); it != insertion_map.end(); it++)
    {
      ofs.write((char*)&(get<0>(it->first)), sizeof(char16_t));
      ofs.write((char*)&(get<1>(it->first)), sizeof(char16_t));
      ofs.write((char*)&(get<2>(it->first)), sizeof(char16_t));
      ofs.write((char*)&(it->second.cost), sizeof(float));
      ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
    }

    uint num_dels = deletion_map.size();
    ofs.write((char*)&num_dels, sizeof(uint32_t));

    for (auto it = deletion_map.begin(); it != deletion_map.end(); it++)
    {
      ofs.write((char*)&(it->first.first), sizeof(char16_t));
      ofs.write((char*)&(it->first.second), sizeof(char16_t));
      ofs.write((char*)&(it->second.cost), sizeof(float));
      ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
    }

    uint num_swaps = swap_map.size();
    ofs.write((char*)&num_swaps, sizeof(uint32_t));

    for (auto it = swap_map.begin(); it != swap_map.end(); it++)
    {
      ofs.write((char*)&(it->first.first), sizeof(char16_t));
      ofs.write((char*)&(it->first.second), sizeof(char16_t));
      ofs.write((char*)&(it->second.cost), sizeof(float));
      ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
    }

  }

  /// @brief Create binary output from text input
  ///
  /// @param text_input Text input
  /// @param binary_output Binary output
  static void CreateBinaryFormFromTextForm(const string &text_input, const string &binary_output)
  {
    ifstream ifs;
    ifs.open(text_input);
    FATAL_CONDITION(ifs.is_open(), "");

    vector<string> toks;
    string s;

    uint min_edit_dist = 100000;

    MyUtils::SafeReadline(ifs, s);
    MyUtils::Split(toks, s, "\t");

    FATAL_CONDITION(toks[0].substr(toks[0].length() - 4) == "case", "");
    FATAL_CONDITION(toks.size() == 3, "");


    ErrorModelOutput case_mismatch = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

    if (min_edit_dist > case_mismatch.edit_dist)
      min_edit_dist = case_mismatch.edit_dist;


    MyUtils::SafeReadline(ifs, s);
    MyUtils::Split(toks, s, "\t");

    FATAL_CONDITION(toks[0] == "substitutions", "");
    FATAL_CONDITION(toks.size() == 3, "");

    ErrorModelOutput sub_default = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

    if (min_edit_dist > sub_default.edit_dist)
      min_edit_dist = sub_default.edit_dist;

    MyUtils::SafeReadline(ifs, s);
    MyUtils::Split(toks, s, "\t");

    FATAL_CONDITION(toks[0] == "insertions", "");
    FATAL_CONDITION(toks.size() == 3, "");

    ErrorModelOutput insert_default = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

    if (min_edit_dist > insert_default.edit_dist)
      min_edit_dist = insert_default.edit_dist;

    MyUtils::SafeReadline(ifs, s);
    MyUtils::Split(toks, s, "\t");

    FATAL_CONDITION(toks[0] == "deletions", "");
    FATAL_CONDITION(toks.size() == 3, "");

    ErrorModelOutput del_default = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

    MyUtils::SafeReadline(ifs, s);
    MyUtils::Split(toks, s, "\t");

    if (min_edit_dist > del_default.edit_dist)
      min_edit_dist = del_default.edit_dist;

    FATAL_CONDITION(toks[0] == "swaps", "");
    FATAL_CONDITION(toks.size() == 3, "");

    ErrorModelOutput swap_default = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

    if (min_edit_dist > swap_default.edit_dist)
      min_edit_dist = swap_default.edit_dist;

    vector<pair<u16string, ErrorModelOutput>> values;

    while (MyUtils::SafeReadline(ifs, s))
    {
      MyUtils::Split(toks, s, "\t");

      if (s == "")
        continue;

      FATAL_CONDITION(toks.size() == 3, s);

      ErrorModelOutput emo = ErrorModelOutput(MyUtils::my_atoi(toks[1]), MyUtils::my_atof(toks[2]));

      if (min_edit_dist > emo.edit_dist)
        min_edit_dist = emo.edit_dist;

      values.push_back(make_pair(MyUTF::utf8_to_utf16(toks[0]), emo));
    }

    cerr << "min_edit_dist = " << min_edit_dist << endl;
    ErrorModelBasic emb(min_edit_dist, case_mismatch, sub_default, insert_default, del_default, swap_default, values);

    ofstream ofs;
    ofs.open(binary_output.c_str(), ios::binary);
    FATAL_CONDITION(ofs.is_open(), "Can't open " << binary_output);

    emb.WriteToStream(ofs);
    ofs.close();
  }

  /// @brief Constructor initialization from input stream.
  ///
  /// @param ifs Input stream
  ErrorModelBasic(istream &ifs)
  {
    ifs.read((char*)&(substitution_default.cost), sizeof(float));
    ifs.read((char*)&(substitution_default.edit_dist), sizeof(uint32_t));
    ifs.read((char*)&(insertion_default.cost), sizeof(float));
    ifs.read((char*)&(insertion_default.edit_dist), sizeof(uint32_t));
    ifs.read((char*)&(deletion_default.cost), sizeof(float));
    ifs.read((char*)&(deletion_default.edit_dist), sizeof(uint32_t));
    ifs.read((char*)&(swap_default.cost), sizeof(float));
    ifs.read((char*)&(swap_default.edit_dist), sizeof(uint32_t));
    ifs.read((char*)&(case_mismatch_cost.cost), sizeof(float));
    ifs.read((char*)&(case_mismatch_cost.edit_dist), sizeof(uint32_t));
    ifs.read((char*)&min_operation_edit_distance, sizeof(uint32_t));

    char16_t ch1;
    char16_t ch2;
    char16_t ch3;
    float cost;
    uint32_t ed_dist;

    uint num_subs;
    ifs.read((char*)&num_subs, sizeof(uint32_t));

    for (uint i = 0; i < num_subs; i++)
    {
      ifs.read((char*)&ch1, sizeof(char16_t));
      ifs.read((char*)&ch2, sizeof(char16_t));
      ifs.read((char*)&cost, sizeof(float));
      ifs.read((char*)&ed_dist, sizeof(uint32_t));

      substitution_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);

      //cerr << "subs: " << ed_dist << " - " << cost << endl;
    }

    uint num_inserts;
    ifs.read((char*)&num_inserts, sizeof(uint32_t));

    for (uint i = 0; i < num_inserts; i++)
    {
      ifs.read((char*)&ch1, sizeof(char16_t));
      ifs.read((char*)&ch2, sizeof(char16_t));
      ifs.read((char*)&ch3, sizeof(char16_t));
      ifs.read((char*)&cost, sizeof(float));
      ifs.read((char*)&ed_dist, sizeof(uint32_t));

      insertion_map[make_tuple(ch1, ch2, ch3)] = ErrorModelOutput(ed_dist, cost);
    }

    uint num_dels;
    ifs.read((char*)&num_dels, sizeof(uint32_t));

    for (uint i = 0; i < num_dels; i++)
    {
      ifs.read((char*)&ch1, sizeof(char16_t));
      ifs.read((char*)&ch2, sizeof(char16_t));
      ifs.read((char*)&cost, sizeof(float));
      ifs.read((char*)&ed_dist, sizeof(uint32_t));

      deletion_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);
    }

    uint num_swaps;
    ifs.read((char*)&num_swaps, sizeof(uint32_t));

    for (uint i = 0; i < num_swaps; i++)
    {
      ifs.read((char*)&ch1, sizeof(char16_t));
      ifs.read((char*)&ch2, sizeof(char16_t));
      ifs.read((char*)&cost, sizeof(float));
      ifs.read((char*)&ed_dist, sizeof(uint32_t));

      swap_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);
    }

  }

  /// @brief Initialize the @ref ErrorModelBasic object from binary file
  ///
  /// @param binary_file File name
  /// @return Pointer ( @ref ErrorModelBasic ) to error model object
  static ErrorModelBasicP fromBinaryFile(string binary_file)
  {
    ifstream ifs;
    ifs.open(binary_file.c_str(), ios::binary);
    FATAL_CONDITION(ifs.is_open(), "");

    ErrorModelBasicP ret = ErrorModelBasicP(new ErrorModelBasic(ifs));
    ifs.close();
    return ret;
  }


  /// @brief Calculate substitution cost
  ///
  /// @param char1 A character
  /// @param char2 A character
  /// @param ignore_case "true" or "false", whether to ignore the case difference
  /// @return An object of type @ref ErrorModelOutput containing the substitution cost and edit distance
  virtual ErrorModelOutput SubstitutionCost(char16_t char1, char16_t char2, bool ignore_case = false)
  {

    if (ignore_case == false)
    {
      auto fit = substitution_map.find(make_pair(char1, char2));
      if (fit == substitution_map.end())
      {
        if (char1 == char2)
          return ErrorModelOutput(0, 0.0f);
        else if (MyUtils::ToLower(char1) == MyUtils::ToLower(char2))
          return case_mismatch_cost;
        else
          return substitution_default;
      }
      else
        return fit->second;
    }
    else
    {
      if (MyUtils::IsUpperCase(char1)) char1 = MyUtils::ToLower(char1);
      if (MyUtils::IsUpperCase(char2)) char2 = MyUtils::ToLower(char2);

      auto fit = substitution_map.find(make_pair(char1, char2));
      if (fit == substitution_map.end())
      {
        if (char1 == char2)
          return ErrorModelOutput(0, 0.0f);
        else
          return substitution_default;
      }
      else
        return fit->second;
    }
  }

  /// @brief Calculate insertion cost
  ///
  /// @param inserted_char Inserted character
  /// @param previous_char Previous character
  /// @param next_char Next character
  /// @return An object of type @ref ErrorModelOutput containing the insertion cost and edit distance
  virtual ErrorModelOutput InsertionCost(char16_t inserted_char, char16_t previous_char, char16_t next_char)
  {
    auto fit = insertion_map.find(make_tuple(inserted_char, previous_char, next_char));
    if (fit == insertion_map.end())
      return insertion_default;
    else
      return fit->second;
  }

  /// @brief Calculate swap cost
  ///
  /// @param first_char A character
  /// @param second_char A character
  /// @return An object of type @ref ErrorModelOutput containing the swap cost and edit distance
  virtual ErrorModelOutput SwapCost(char16_t first_char, char16_t second_char)
  {
    auto fit = swap_map.find(make_pair(first_char, second_char));

    if (fit == swap_map.end())
      return swap_default;
    else
      return fit->second;
  }

  /// @brief Calculate deletion cost
  ///
  /// @param current_char Current character
  /// @param previous_char Previous character
  /// @return An object of type @ref ErrorModelOutput containing the deletion cost and edit distance
  virtual ErrorModelOutput DeletionCost(char16_t current_char, char16_t previous_char)
  {
    auto fit = deletion_map.find(make_pair(current_char, previous_char));
    if (fit == deletion_map.end())
      return deletion_default;
    else
      return fit->second;
  }

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

}
#endif //_ERROR_MODEL_BASIC_HPP
