#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <stdint.h>


#include <utils.hpp>

#ifndef uint
#define uint uint32_t
#endif

using namespace std;
using namespace ngramchecker;

struct factor_description
{
  string factor_name;
  uint vocab_size;
  float branching_factor;
  bool next_level_grouped;
  bool is_grouped;
  uint group_num_elements;
  uint num_groups;
  bool branching;
  bool is_governed;
  string governing_factor;
  uint governing_factor_id;

  vector<string> vocab;
  vector<vector<uint> > groups;

  vector<uint> governance_mapping;

  factor_description(): factor_name(""), vocab_size(0), branching_factor(0),
  next_level_grouped(false), is_grouped(false), group_num_elements(0), num_groups(0), branching(true),
  is_governed(false), governing_factor("") {}

  void init(vector<factor_description> &factors)
  {
    vocab.clear();
    groups.clear();

    for (uint j = 0; j < vocab_size; j++)
    {
      vocab.push_back(factor_name + "_" + MyUtils::my_itoa(j));
    }

    if (is_grouped)
    {
      for (uint i = 0; i < num_groups; i++)
      {

        set<uint> group_set;
        for (uint j = 0; j < group_num_elements; j++)
        {
          uint factorID = MyUtils::randomR(0, vocab_size);

          if (group_set.find(factorID) == group_set.end())
            group_set.insert(factorID);
        }

        vector<uint> new_group = vector<uint>(group_set.begin(), group_set.end());
        groups.push_back(new_group);
      }
    }

    if (is_governed)
    {
      for (uint i = 0; i < factors[governing_factor_id].vocab.size(); i++)
      {
        if (is_grouped == false)
        {
          uint factorID = MyUtils::randomR(0, vocab_size);
          governance_mapping.push_back(factorID);
        }
        else
        {
          uint groupID = MyUtils::randomR(0, num_groups);
          governance_mapping.push_back(groupID);
        }
      }
    }
  }
};

map<string, uint> factor_names_map;
vector<factor_description> factors;

void create_morphology_rec(uint level, uint factor_to_be_added, vector<uint> &outline, vector<string> &morphology_lines)
{
  outline.push_back(factor_to_be_added);

  if (level + 1 < factors.size())
  {
    if (factors[level + 1].is_grouped == true)
    {
      uint groupID;
      if (factors[level + 1].is_governed == true)
        groupID = factors[level + 1].governance_mapping[outline[factors[level + 1].governing_factor_id]];
      else
        groupID = MyUtils::randomR(0, factors[level + 1].num_groups);

      for (uint i = 0; i < factors[level + 1].groups[groupID].size(); i++)
        create_morphology_rec(level + 1, factors[level + 1].groups[groupID][i], outline, morphology_lines);
    }
    else
    {
      if (factors[level + 1].is_governed == false)
      {
        uint num_children;
        if (factors[level].branching == false)
          num_children = 1;
        else
        {
          float rest = factors[level].branching_factor - (uint)(factors[level].branching_factor);
          float rand01 = MyUtils::RandomNumber(0.0, 1.0);

          if (rand01 < rest)
            num_children = (uint)(factors[level].branching_factor);
          else
            num_children = (uint)(factors[level].branching_factor) + 1;
        }

        for (uint i = 0; i < num_children; i++)
        {
          uint factorID = MyUtils::randomR(0, factors[level + 1].vocab_size);
          create_morphology_rec(level + 1, factorID, outline, morphology_lines);
        }
      }
      else
      {
        uint factorID = factors[level + 1].governance_mapping[outline[factors[level + 1].governing_factor_id]];
        create_morphology_rec(level + 1, factorID, outline, morphology_lines);
      }
    }

  }
  else
  {
    stringstream strs;
    for (uint i = 0; i < outline.size(); i++)
    {
      if (i > 0) strs << "|";
      strs << factors[i].vocab[outline[i]];
    }

    morphology_lines.push_back(strs.str());
  }


  outline.pop_back();
}

int main(int argc, char** argv)
{


  FATAL_CONDITION(argc == 4, "expected parameters: [input: morphology_description_file] [output: morphology_output] [output: corpus_output]");

  ifstream ifs;

  ifs.open(argv[1], ios::in);

  FATAL_CONDITION(ifs.is_open(), "the file " << argv[1] << "cannot be opened!");

  factor_description fd;
  string s;

  while (MyUtils::SafeReadline(ifs, s))
  {
    if (s == "---")
    {
      factors.push_back(fd);
      fd = factor_description();
    }
    else if (s == "no_branching")
    {
      fd.branching = false;
    }
    else if (s == "next_level_grouped")
    {
      fd.next_level_grouped = true;
    }
    else if (s == "grouped")
    {
      fd.is_grouped = true;
    }
    else
    {
      vector<string> parts;
      MyUtils::Split(parts, s, "=");

      if (parts.size() != 2)
      {
        cerr << "error at line: " << s << std::endl;
        return -3;
      }

      if (parts[0] == "factor_name")
      {
        fd.factor_name = parts[1];
        factor_names_map[parts[1]] = factors.size();
      }
      else if (parts[0] == "vocab_size")
        fd.vocab_size = MyUtils::my_atoi(parts[1]);
      else if (parts[0] == "branching_factor")
        fd.branching_factor = MyUtils::my_atof(parts[1]);
      else if (parts[0] == "num_groups")
        fd.num_groups = MyUtils::my_atoi(parts[1]);
      else if (parts[0] == "group_num_elements")
        fd.group_num_elements = MyUtils::my_atoi(parts[1]);
      else if (parts[0] == "governing_factor")
      {
        fd.governing_factor = parts[1];
        fd.is_governed = true;
        fd.governing_factor_id = factor_names_map[parts[1]];
      }
      else
      {
        cerr << "definition file error: " << s << endl;
        return -4;
      }
    }
  }

  for (uint i = 0; i < factors.size(); i++)
    factors[i].init(factors);

  ofstream ofs;
  ofs.open(argv[2], ios::out);
  FATAL_CONDITION(ofs.is_open(), "the file " << argv[2] << "cannot be opened");


  vector<string> morphology_lines;

  for (uint i = 0; i < factors[0].vocab_size; i++)
  {
    vector<uint> outline;
    uint level = 0;

    create_morphology_rec(level, i, outline, morphology_lines);
  }

  for (uint i = 0; i < morphology_lines.size(); i++)
    ofs << morphology_lines[i] << endl;

  ifs.close();
  ofs.close();

  ofs.open(argv[3], ios::out);

  FATAL_CONDITION(ofs.is_open(), "the file " << argv[3] << "can't be opened");

  for (uint i = 0; i < factors.size(); i++)
  {
    if (i > 0) ofs << "|";
    ofs << factors[i].factor_name;
  }

  ofs << endl;

  for (uint i = 0; i < 1000; i++)
  {
    uint sentence_length = 5 + abs(rand()) % 10;

    for (uint j = 0; j < sentence_length; j++)
    {
      uint entry_index = MyUtils::randomR(0, morphology_lines.size());

      if (j > 0) ofs << " ";
      ofs << morphology_lines[entry_index];
    }
    ofs << "\n";
  }

  ofs.close();

  cerr << "OK!";
  cin >> s;
}
