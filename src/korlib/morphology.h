// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file morphology.h
/// @class Morphology morphology.h "morphology.h"
/// @brief Class for loading, accessing, manipulating morphology lexicon.
///
/// This class provides various functions to access the morphology data from file.
/// @todo PrintOut function should be traced thoroughly

#pragma once

#include "common.h"
#include "my_packed_array.h"
#include "comp_increasing_array.h"
#include "my_bit_array.h"
#include "my_static_string_array.h"
#include "value_mapping.h"
#include "stage_posibility.h"
#include "configuration.h"
#include "factor_list.h"

namespace ufal {
namespace korektor {

//for some factors, there are a lot of possibilities per word, however the number of sets of values that can be assigned to a word is limited
//for example: in Czech language many word forms can be either first person singular or first person plural - i.e. these two posibilities form a set that tends to reocur
//another example: adjectives like 'jarní' can stand for all cases in both singular and plural per each gender - this forms a reocuring set of more than 40 possibilities
//for some factors, it is more efficient to collect descriptors of all groups and specify just a group ID per word rather than the list of factor values
//morpho_grpuping struct contains definition of such grouped factor - i.e. which group contains which factors
struct morpho_grouping {
  MyPackedArray group_values;
  CompIncreasingArray group_offsets;

  inline void getGroupMembers(unsigned groupID, vector<unsigned> &group_members)
  {

    group_members.clear();
    CompIA_First_Last_IndexPair index_pair = group_offsets.GetFirstLastIndexPair(groupID);
    for (unsigned i = index_pair.first; i <= index_pair.second; i++)
      group_members.push_back(group_values.GetValueAt(i));
  }

  morpho_grouping(const MyPackedArray &_group_values, const CompIncreasingArray &_group_offsets):
    group_values(_group_values), group_offsets(_group_offsets) {}
};

SP_DEF(morpho_grouping);

//morpho_dependency captures the situation when the value of one factor determines the value of another factor
//for example, full morphological tag (let's say NNS6---------) determines the value of short morphological tag (NN - just the starting letter of factors)
//or a word form could determine a lemma (although there are usually exceptions)
//in such cases - morpho_dependency represents the mapping between governing factor values and the governed factor values
struct morpho_dependency {
  unsigned governing_factor;
  MyPackedArray dependency_mapping;

  inline unsigned GetValue(unsigned governing_factor_id)
  {
    return dependency_mapping.GetValueAt(governing_factor_id);
  }

  inline unsigned GetGoverningFactorIndex()
  {
    return governing_factor;
  }

  morpho_dependency(unsigned _governing_factor, const MyPackedArray &dep_mapping): governing_factor(_governing_factor), dependency_mapping(dep_mapping) {}
};

SP_DEF(morpho_dependency);


#if 0
struct MorphoNode;
SP_DEF(MorphoNode);

//TODO: well this is no good! Keep it simple! :)
struct MorphoNode {
  unsigned level;
  unsigned factorID;
  MorphoNodeP parent;
  double emission_cost;

  MorphoNode(unsigned _level, unsigned _factorID): level(_level), factorID(_factorID) {}
  MorphoNode(unsigned _level): level(_level) {}
};
#endif


class Morphology {
  uint32_t num_factors; ///< Number of factors
  map<string, unsigned> factor_names; ///< Factor names
  vector<unsigned> bits_per_value; ///< Number of bits needed to store a factorID for the particular factor
  vector<unsigned> bits_per_children; ///< Number of bits needed to store a number of children for a node at the particular level or a groupID if the level is grouped
  vector<morpho_dependencyP> dependencies; ///< List of all factor dependencies
  vector<morpho_groupingP> groupings;

  ValueMapping value_mapping; ///< Probability values mapping used for obtaining emission probabilities, i.e. (mapping between packed representation of probs used in morphoData and normal floating numbers)

  vector<unsigned> group_members_pom;

  CompIncreasingArray formOffsets; ///< Contains offsets into the morphoData bit array denoting where the morphological information for a particular factor starts

  MyBitArray morphoData; ///< Bit array containing the morphological information

  //morpho_word_lists and morpho_maps suits only debuggind purposes and usually are not loaded at all
  vector<MyStaticStringArrayP> morpho_word_lists;
  vector<map<string, unsigned> > morpho_maps;


  unsigned last_enabled_factor;

 private:

  void get_morphology_rec(unsigned level, FactorList &flist, vector<FactorList> &ret, unsigned& bit_offset, Configuration *configuration, int next_factor = -1)
  {
    FATAL_CONDITION(level < FactorList::MAX_FACTORS, "Too many factors encountered!");

    bool current_level_enabled = configuration->FactorIsEnabled(level);


    if (level > 0)
    {
      if (current_level_enabled)
        flist.emission_costs[level] = value_mapping.GetDouble(morphoData.GetValueAt(bit_offset, value_mapping.BitsPerValue()));
      //node->emission_cost = value_mapping.GetDouble(morphoData.GetValueAt(bit_offset, value_mapping.BitsPerValue()));
      bit_offset += value_mapping.BitsPerValue();
    }

    if (next_factor >= 0)
    {
      flist.factors[level] = (unsigned)next_factor;
      //curr_factors.push_back((unsigned)next_factor);
      //node->factorID = (unsigned)next_factor;
    }
    else if (dependencies[level])
    {
      unsigned cur_fact = dependencies[level]->GetValue(flist.factors[dependencies[level]->GetGoverningFactorIndex()]);
      flist.factors[level] = cur_fact;
      //node->factorID = cur_fact;
      //curr_factors.push_back(cur_fact);
    }
    else if (level > 0)
    {
      unsigned cur_fact = morphoData.GetValueAt(bit_offset, bits_per_value[level]);
      flist.factors[level] = cur_fact;
      //curr_factors.push_back(cur_fact);
      //node->factorID = cur_fact;
      bit_offset += bits_per_value[level];
    }

    if (level + 1 < num_factors)
    {

      //bool next_level_enabled = configuration->FactorIsEnabled(level + 1);

      if (!(dependencies[level + 1]))
      {

        if (groupings[level + 1])
        {
          unsigned groupID = morphoData.GetValueAt(bit_offset, bits_per_children[level]);
          bit_offset += bits_per_children[level];

          vector<unsigned> group_members;
          groupings[level + 1]->getGroupMembers(groupID, group_members);
          for (unsigned i = 0; i < group_members.size(); i++)
          {
            get_morphology_rec(level + 1, flist, ret, bit_offset, configuration, group_members[i]);
          }
        }
        else
        {
          unsigned num_children = morphoData.GetValueAt(bit_offset, bits_per_children[level]);
          num_children++; //there is never zero children so the values are shifted
          bit_offset += bits_per_children[level];

          for (unsigned i = 0; i < num_children; i++)
          {
            get_morphology_rec(level + 1, flist, ret, bit_offset, configuration);
          }
        }
      }
      else
      {
        get_morphology_rec(level + 1, flist, ret, bit_offset, configuration);
      }

    }

    if (level == configuration->GetLastEnabledFactorIndex()) ret.push_back(flist);
  }

 public:

  /// @brief Get the factor map
  ///
  /// @return Hash map containing factor names as keys and indices as values
  map<string, unsigned>& GetFactorMap()
  {
    return factor_names;
  }

  /// @brief Get the factor string
  ///
  /// @return Factor string
  string GetFactorString(unsigned factor_index, unsigned ID)
  {
    return morpho_word_lists[factor_index]->GetStringAt(ID);
  }

  void initMorphoWordMaps()
  {
    morpho_maps.clear();
    assert(morpho_word_lists.size() > 0);

    for (auto it = morpho_word_lists.begin(); it != morpho_word_lists.end(); it++)
    {
      morpho_maps.push_back(map<string, unsigned>());

      for (unsigned i = 0; i < (*it)->GetSize(); i++)
      {
        morpho_maps.back().insert(make_pair((*it)->GetStringAt(i), i));
      }
    }
  }

  int GetFactorID(unsigned factor_index, const string &str)
  {
    if (morpho_maps.size() == 0)
      initMorphoWordMaps();
    assert(factor_index < morpho_maps.size());

    auto fit = morpho_maps[factor_index].find(str);

    if (fit == morpho_maps[factor_index].end())
      return -1;
    else
      return fit->second;

  }

  void initMorphoWordLists(string filename)
  {
    morpho_word_lists.clear();
    ifstream ifs;
    ifs.open(filename.c_str(), ios::in | ios::binary);
    if (!ifs.is_open()) {
      cerr << "Opening morpholex vocabulary file " << filename << " failed!" << endl;
      exit(1);
    }

    uint32_t num_f;
    ifs.read((char*)&num_f, sizeof(uint32_t));
    assert(num_f == num_factors);

    for (unsigned i = 0; i < num_factors; i++)
    {
      morpho_word_lists.push_back(MyStaticStringArrayP(new MyStaticStringArray(ifs)));
      cerr << "initializating word lists: " << i << " : " << morpho_word_lists[i]->GetSize() << " entries" << endl;
    }
  }

  /// @brief Get the morphology for a particular form
  ///
  /// @param form_id Word form id
  /// @param configuration Pointer to the instance of @ref Configuration class
  /// @return Factor list
  vector<FactorList> GetMorphology(unsigned form_id, Configuration* configuration)
  {
    vector<FactorList> ret;
    FactorList flist;
    memset(&flist, 0, sizeof(FactorList));

    flist.factors[0] = form_id;

    if (form_id >= formOffsets.GetSize())
      return ret;

    unsigned bit_offset = formOffsets.GetValueAt(form_id);

    get_morphology_rec(0, flist, ret, bit_offset, configuration);

    return ret;
  }

  /// @brief Print out morphology
  ///
  /// @param ofs Output stream
  /// @param configuration Pointer to the instance of @ref Configuration class
  void PrintOut(ostream &ofs, Configuration* configuration)
  {
    FATAL_CONDITION(morpho_word_lists.size() > 0, "");

    for (unsigned i = 0; i < formOffsets.GetSize(); i++)
    {
      //cerr << "printing out node: " << i << endl;
      vector<FactorList> morpho_anal = GetMorphology(i, configuration);

      for (unsigned j = 0; j < morpho_anal.size(); j++)
      {
        FactorList flist = morpho_anal[j];
        vector<string> strs;

        for (unsigned k = 0; k < configuration->GetLastEnabledFactorIndex(); k++)
        {
          if (configuration->FactorIsEnabled(k))
            strs.push_back(morpho_word_lists[k]->GetStringAt(flist.factors[k]));
        }

        for (int k = (int)strs.size() - 1; k >= 0; k--)
        {
          if ((unsigned)k < strs.size() - 1) ofs << "|";
          ofs << strs[k];
        }
        ofs << endl;
      }

    }

    cerr << "Printing out morphology succesfully finished!" << endl;
  }

  /// @brief Morphology constructor initialization. Reads morph data from file.
  ///
  /// @param ifs Input stream
  Morphology(ifstream &ifs)
  {
    string checkIT = MyUtils::ReadString(ifs);

    FATAL_CONDITION(checkIT == "Morphology", "");

    ifs.read((char*)&num_factors, sizeof(uint32_t));

    FATAL_CONDITION(num_factors <= FactorList::MAX_FACTORS, "Too many factors!");
    for (size_t i = 0; i < num_factors; i++)
    {
      dependencies.push_back(morpho_dependencyP());
      groupings.push_back(morpho_groupingP());
    }

    MyStaticStringArray factor_mssa = MyStaticStringArray(ifs);

    for (unsigned i = 0; i < factor_mssa.GetSize(); i++)
    {
      factor_names[factor_mssa.GetStringAt(i)] = i;
    }


    uint32_t num_dependencies;
    ifs.read((char*)&num_dependencies, sizeof(uint32_t));

    assert(num_dependencies < num_factors);

    for (unsigned i = 0; i < num_dependencies; i++)
    {
      uint32_t governing;
      uint32_t governed;
      ifs.read((char*)&governing, sizeof(uint32_t));
      ifs.read((char*)&governed, sizeof(uint32_t));

      assert(governing < governed);

      //MyPackedArrayP dep_array = MyPackedArrayP(new MyPackedArray(ifs));

      dependencies[governed] = morpho_dependencyP(new morpho_dependency(governing, MyPackedArray(ifs)));
    }

    uint32_t num_grouped;
    ifs.read((char*)&num_grouped, sizeof(uint32_t));

    assert(num_grouped < num_factors);

    for (unsigned i = 0; i < num_grouped; i++)
    {
      uint32_t factor_index;
      ifs.read((char*)&factor_index, sizeof(uint32_t));

      CompIncreasingArray group_offsets = CompIncreasingArray(ifs);
      MyPackedArray group_values = MyPackedArray(ifs);

      groupings[factor_index] = morpho_groupingP(new morpho_grouping(group_values, group_offsets));
    }


    for (unsigned i = 0; i < num_factors; i++)
    {
      uint32_t bpv;
      ifs.read((char*)&bpv, sizeof(uint32_t));
      assert(bpv < 64);
      bits_per_value.push_back(bpv);
    }

    for (unsigned i = 0; i < num_factors - 1; i++)
    {
      uint32_t bpch;
      ifs.read((char*)&bpch, sizeof(uint32_t));
      assert(bpch < 64);
      bits_per_children.push_back(bpch);
    }

    value_mapping = ValueMapping(ifs);

    formOffsets = CompIncreasingArray(ifs);
    morphoData = MyBitArray(ifs);
  }

  /// @brief Write the morphology to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs)
  {
    MyUtils::WriteString(ofs, "Morphology");
    ofs.write((char*)&num_factors, sizeof(uint32_t));

    vector<string> factor_names_vec;
    factor_names_vec.resize(num_factors);

    for (auto it = factor_names.begin(); it != factor_names.end(); it++)
    {
      factor_names_vec[it->second] = it->first;
    }

    MyStaticStringArray mssa = MyStaticStringArray(factor_names_vec);
    mssa.WriteToStream(ofs);

    unsigned num_dependencies = 0;
    for (auto it = dependencies.begin(); it != dependencies.end(); it++)
      if (*it) num_dependencies++;

    ofs.write((char*)&num_dependencies, sizeof(uint32_t));

    for (unsigned i = 0; i < dependencies.size(); i++)
    {
      morpho_dependencyP mp = dependencies[i];

      if (!mp) continue;

      uint32_t governing = mp->governing_factor;
      uint32_t governed = i;
      ofs.write((char*)&governing, sizeof(uint32_t));
      ofs.write((char*)&governed, sizeof(uint32_t));

      mp->dependency_mapping.WriteToStream(ofs);
    }

    unsigned num_grouped = 0;
    for (auto it = groupings.begin(); it != groupings.end(); it++)
      if (*it) num_grouped++;

    ofs.write((char*)&num_grouped, sizeof(uint32_t));

    for (unsigned i = 0; i < groupings.size(); i++)
    {
      if (! (groupings[i])) continue;
      morpho_groupingP mgp = groupings[i];

      ofs.write((char*)&i, sizeof(uint32_t));

      mgp->group_offsets.WriteToStream(ofs);
      mgp->group_values.WriteToStream(ofs);
    }

    for (unsigned i = 0; i < bits_per_value.size(); i++)
    {
      ofs.write((char*)&(bits_per_value[i]), sizeof(uint32_t));
    }

    for (unsigned i = 0; i < bits_per_children.size(); i++)
    {
      ofs.write((char*)&(bits_per_children[i]), sizeof(uint32_t));
    }

    value_mapping.writeToStream(ofs);

    formOffsets.WriteToStream(ofs);

    morphoData.WriteToStream(ofs);


  }

};

SP_DEF(Morphology);

} // namespace korektor
} // namespace ufal
