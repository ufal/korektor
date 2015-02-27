// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <fstream>
#include <iostream>

#include "error_model_basic.h"
#include "utils/io.h"
#include "utils/parse.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

ErrorModelBasic::ErrorModelBasic(unsigned _min_operation_edit_distance, ErrorModelOutput _case_mismatch_cost,
                                 ErrorModelOutput _substitution_default, ErrorModelOutput _insertion_default,
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

    if (signature.substr(0, 2) == UTF::UTF8To16("s_"))
    {
      char16_t ch1 = signature[2];
      char16_t ch2 = signature[3];

      substitution_map[make_pair(ch1, ch2)] = emo;

      char16_t ch1_uc = UTF::ToUpper(ch1);
      char16_t ch2_uc = UTF::ToUpper(ch2);

      substitution_map[make_pair(ch1_uc, ch2_uc)] = emo;
      substitution_map[make_pair(ch1_uc, ch2)] = emo_case_mismatch;
      substitution_map[make_pair(ch1, ch2_uc)] = emo_case_mismatch;
    }
    else if (signature.substr(0, 5) == UTF::UTF8To16("swap_"))
    {
      char16_t ch1 = signature[5];
      char16_t ch2 = signature[6];

      swap_map[make_pair(ch1, ch2)] = emo;

      char16_t ch1_uc = UTF::ToUpper(ch1);
      char16_t ch2_uc = UTF::ToUpper(ch2);

      swap_map[make_pair(ch1_uc, ch2_uc)] = emo;
      swap_map[make_pair(ch1_uc, ch2)] = emo;
      swap_map[make_pair(ch1, ch2_uc)] = emo;

    }
    else if (signature.substr(0, 2) == UTF::UTF8To16("i_"))
    {
      char16_t ch1 = signature[2];
      char16_t ch2 = signature[3];
      char16_t ch3 = signature[4];

      insertion_map[make_tuple(ch1, ch2, ch3)] = emo;

      char16_t ch1_uc = UTF::ToUpper(ch1);
      char16_t ch2_uc = UTF::ToUpper(ch2);
      char16_t ch3_uc = UTF::ToUpper(ch3);

      insertion_map[make_tuple(ch1_uc, ch2, ch3)] = emo;
      insertion_map[make_tuple(ch1, ch2_uc, ch3)] = emo;
      insertion_map[make_tuple(ch1_uc, ch2_uc, ch3)] = emo;
      insertion_map[make_tuple(ch1, ch2, ch3_uc)] = emo;
      insertion_map[make_tuple(ch1_uc, ch2, ch3_uc)] = emo;
      insertion_map[make_tuple(ch1, ch2_uc, ch3_uc)] = emo;
      insertion_map[make_tuple(ch1_uc, ch2_uc, ch3_uc)] = emo;
    }
    else if (signature.substr(0, 2) == UTF::UTF8To16("d_"))
    {
      char16_t ch1 = signature[2];
      char16_t ch2 = signature[3];

      deletion_map[make_pair(ch1, ch2)] = emo;

      char16_t ch1_uc = UTF::ToUpper(ch1);
      char16_t ch2_uc = UTF::ToUpper(ch2);

      deletion_map[make_pair(ch1_uc, ch2)] = emo;
      deletion_map[make_pair(ch1, ch2_uc)] = emo;
      deletion_map[make_pair(ch1_uc, ch2_uc)] = emo;
    }
  }
}

void ErrorModelBasic::WriteToStream(ostream &ofs)
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

  unsigned num_subs = substitution_map.size();
  ofs.write((char*)&num_subs, sizeof(uint32_t));

  for (auto it = substitution_map.begin(); it != substitution_map.end(); it++)
  {
    ofs.write((char*)&(it->first.first), sizeof(char16_t));
    ofs.write((char*)&(it->first.second), sizeof(char16_t));
    ofs.write((char*)&(it->second.cost), sizeof(float));
    ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
  }

  unsigned num_inserts = insertion_map.size();
  ofs.write((char*)&num_inserts, sizeof(uint32_t));

  for (auto it = insertion_map.begin(); it != insertion_map.end(); it++)
  {
    ofs.write((char*)&(get<0>(it->first)), sizeof(char16_t));
    ofs.write((char*)&(get<1>(it->first)), sizeof(char16_t));
    ofs.write((char*)&(get<2>(it->first)), sizeof(char16_t));
    ofs.write((char*)&(it->second.cost), sizeof(float));
    ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
  }

  unsigned num_dels = deletion_map.size();
  ofs.write((char*)&num_dels, sizeof(uint32_t));

  for (auto it = deletion_map.begin(); it != deletion_map.end(); it++)
  {
    ofs.write((char*)&(it->first.first), sizeof(char16_t));
    ofs.write((char*)&(it->first.second), sizeof(char16_t));
    ofs.write((char*)&(it->second.cost), sizeof(float));
    ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
  }

  unsigned num_swaps = swap_map.size();
  ofs.write((char*)&num_swaps, sizeof(uint32_t));

  for (auto it = swap_map.begin(); it != swap_map.end(); it++)
  {
    ofs.write((char*)&(it->first.first), sizeof(char16_t));
    ofs.write((char*)&(it->first.second), sizeof(char16_t));
    ofs.write((char*)&(it->second.cost), sizeof(float));
    ofs.write((char*)&(it->second.edit_dist), sizeof(uint32_t));
  }
}

void ErrorModelBasic::CreateBinaryFormFromTextForm(const string &text_input, const string &binary_output)
{
  ifstream ifs;
  ifs.open(text_input);
  if (!ifs.is_open())
    runtime_errorf("Cannot open file '%s'!", text_input.c_str());

  vector<string> toks;
  string s;

  unsigned min_edit_dist = 100000;

  IO::ReadLine(ifs, s);
  IO::Split(s, '\t', toks);

  if (toks.size() != 3)
    runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());
  if (toks[0] != "case")
    runtime_errorf("Expected to see 'case' on line '%s' in file '%s'!", s.c_str(), text_input.c_str());


  ErrorModelOutput case_mismatch = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

  if (min_edit_dist > case_mismatch.edit_dist)
    min_edit_dist = case_mismatch.edit_dist;


  IO::ReadLine(ifs, s);
  IO::Split(s, '\t', toks);

  if (toks.size() != 3)
    runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());
  if (toks[0] != "substitutions")
    runtime_errorf("Expected to see 'substitutions' on line '%s' in file '%s'!", s.c_str(), text_input.c_str());

  ErrorModelOutput sub_default = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

  if (min_edit_dist > sub_default.edit_dist)
    min_edit_dist = sub_default.edit_dist;

  IO::ReadLine(ifs, s);
  IO::Split(s, '\t', toks);

  if (toks.size() != 3)
    runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());
  if (toks[0] != "insertions")
    runtime_errorf("Expected to see 'insertions' on line '%s' in file '%s'!", s.c_str(), text_input.c_str());

  ErrorModelOutput insert_default = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

  if (min_edit_dist > insert_default.edit_dist)
    min_edit_dist = insert_default.edit_dist;

  IO::ReadLine(ifs, s);
  IO::Split(s, '\t', toks);

  if (toks.size() != 3)
    runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());
  if (toks[0] != "deletions")
    runtime_errorf("Expected to see 'deletions' on line '%s' in file '%s'!", s.c_str(), text_input.c_str());

  ErrorModelOutput del_default = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

  IO::ReadLine(ifs, s);
  IO::Split(s, '\t', toks);

  if (min_edit_dist > del_default.edit_dist)
    min_edit_dist = del_default.edit_dist;

  if (toks.size() != 3)
    runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());
  if (toks[0] != "swaps")
    runtime_errorf("Expected to see 'swaps' on line '%s' in file '%s'!", s.c_str(), text_input.c_str());

  ErrorModelOutput swap_default = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

  if (min_edit_dist > swap_default.edit_dist)
    min_edit_dist = swap_default.edit_dist;

  vector<pair<u16string, ErrorModelOutput>> values;

  while (IO::ReadLine(ifs, s))
  {
    IO::Split(s, '\t', toks);

    if (s == "")
      continue;

    if (toks.size() != 3)
      runtime_errorf("Not three columns on line '%s' in file '%s'!", s.c_str(), text_input.c_str());

    ErrorModelOutput emo = ErrorModelOutput(Parse::Int(toks[1], "edit distance"), Parse::Double(toks[2], "error model operation cost"));

    if (min_edit_dist > emo.edit_dist)
      min_edit_dist = emo.edit_dist;

    values.push_back(make_pair(UTF::UTF8To16(toks[0]), emo));
  }

  cerr << "min_edit_dist = " << min_edit_dist << endl;
  ErrorModelBasic emb(min_edit_dist, case_mismatch, sub_default, insert_default, del_default, swap_default, values);

  ofstream ofs;
  ofs.open(binary_output.c_str(), ios::binary);
  if (!ofs.is_open())
    runtime_errorf("Cannot open file '%s'!", binary_output.c_str());

  emb.WriteToStream(ofs);
  ofs.close();
}

ErrorModelBasic::ErrorModelBasic(istream &ifs)
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

  uint32_t num_subs;
  ifs.read((char*)&num_subs, sizeof(uint32_t));

  for (unsigned i = 0; i < num_subs; i++)
  {
    ifs.read((char*)&ch1, sizeof(char16_t));
    ifs.read((char*)&ch2, sizeof(char16_t));
    ifs.read((char*)&cost, sizeof(float));
    ifs.read((char*)&ed_dist, sizeof(uint32_t));

    substitution_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);

    //cerr << "subs: " << ed_dist << " - " << cost << endl;
  }

  uint32_t num_inserts;
  ifs.read((char*)&num_inserts, sizeof(uint32_t));

  for (unsigned i = 0; i < num_inserts; i++)
  {
    ifs.read((char*)&ch1, sizeof(char16_t));
    ifs.read((char*)&ch2, sizeof(char16_t));
    ifs.read((char*)&ch3, sizeof(char16_t));
    ifs.read((char*)&cost, sizeof(float));
    ifs.read((char*)&ed_dist, sizeof(uint32_t));

    insertion_map[make_tuple(ch1, ch2, ch3)] = ErrorModelOutput(ed_dist, cost);
  }

  uint32_t num_dels;
  ifs.read((char*)&num_dels, sizeof(uint32_t));

  for (unsigned i = 0; i < num_dels; i++)
  {
    ifs.read((char*)&ch1, sizeof(char16_t));
    ifs.read((char*)&ch2, sizeof(char16_t));
    ifs.read((char*)&cost, sizeof(float));
    ifs.read((char*)&ed_dist, sizeof(uint32_t));

    deletion_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);
  }

  uint32_t num_swaps;
  ifs.read((char*)&num_swaps, sizeof(uint32_t));

  for (unsigned i = 0; i < num_swaps; i++)
  {
    ifs.read((char*)&ch1, sizeof(char16_t));
    ifs.read((char*)&ch2, sizeof(char16_t));
    ifs.read((char*)&cost, sizeof(float));
    ifs.read((char*)&ed_dist, sizeof(uint32_t));

    swap_map[make_pair(ch1, ch2)] = ErrorModelOutput(ed_dist, cost);
  }

}

ErrorModelBasicP ErrorModelBasic::fromBinaryFile(string binary_file)
{
  ifstream ifs;
  ifs.open(binary_file.c_str(), ios::binary);
  if (!ifs.is_open())
    runtime_errorf("Cannot open file '%s'!", binary_file.c_str());

  ErrorModelBasicP ret = ErrorModelBasicP(new ErrorModelBasic(ifs));
  ifs.close();
  return ret;
}

ErrorModelOutput ErrorModelBasic::SubstitutionCost(char16_t char1, char16_t char2, bool ignore_case)
{
  if (ignore_case == false)
  {
    auto fit = substitution_map.find(make_pair(char1, char2));
    if (fit == substitution_map.end())
    {
      if (char1 == char2)
        return ErrorModelOutput(0, 0.0f);
      else if (UTF::ToLower(char1) == UTF::ToLower(char2))
        return case_mismatch_cost;
      else
        return substitution_default;
    }
    else
      return fit->second;
  }
  else
  {
    char1 = UTF::ToLower(char1);
    char2 = UTF::ToLower(char2);

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

ErrorModelOutput ErrorModelBasic::InsertionCost(char16_t inserted_char, char16_t previous_char, char16_t next_char)
{
  auto fit = insertion_map.find(make_tuple(inserted_char, previous_char, next_char));
  if (fit == insertion_map.end())
    return insertion_default;
  else
    return fit->second;
}

ErrorModelOutput ErrorModelBasic::SwapCost(char16_t first_char, char16_t second_char)
{
  auto fit = swap_map.find(make_pair(first_char, second_char));

  if (fit == swap_map.end())
    return swap_default;
  else
    return fit->second;
}

ErrorModelOutput ErrorModelBasic::DeletionCost(char16_t current_char, char16_t previous_char)
{
  auto fit = deletion_map.find(make_pair(current_char, previous_char));
  if (fit == deletion_map.end())
    return deletion_default;
  else
    return fit->second;
}

} // namespace korektor
} // namespace ufal
