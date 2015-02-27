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

#include <unordered_map>

#include "common.h"
#include "bit_array.h"
#include "comp_increasing_array.h"
#include "factor_list.h"
#include "string_array.h"
#include "value_mapping.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

struct morpho_grouping;
SP_DEF(morpho_grouping);

struct morpho_dependency;
SP_DEF(morpho_dependency);


class Morphology {
  uint32_t num_factors; ///< Number of factors
  unordered_map<string, unsigned> factor_names; ///< Factor names
  vector<unsigned> bits_per_value; ///< Number of bits needed to store a factorID for the particular factor
  vector<unsigned> bits_per_children; ///< Number of bits needed to store a number of children for a node at the particular level or a groupID if the level is grouped
  vector<morpho_dependencyP> dependencies; ///< List of all factor dependencies
  vector<morpho_groupingP> groupings;

  ValueMapping value_mapping; ///< Probability values mapping used for obtaining emission probabilities, i.e. (mapping between packed representation of probs used in morphoData and normal floating numbers)

  vector<unsigned> group_members_pom;

  CompIncreasingArray formOffsets; ///< Contains offsets into the morphoData bit array denoting where the morphological information for a particular factor starts

  BitArray morphoData; ///< Bit array containing the morphological information

  //morpho_word_lists and morpho_maps suits only debuggind purposes and usually are not loaded at all
  vector<StringArrayP> morpho_word_lists;
  vector<unordered_map<string, unsigned> > morpho_maps;


  unsigned last_enabled_factor;

 private:

  void get_morphology_rec(unsigned level, FactorList &flist, vector<FactorList> &ret, unsigned& bit_offset, Configuration *configuration, int next_factor = -1);

 public:

  /// @brief Get the factor map
  ///
  /// @return Hash map containing factor names as keys and indices as values
  unordered_map<string, unsigned>& GetFactorMap()
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

  void initMorphoWordMaps();

  int GetFactorID(unsigned factor_index, const string &str);

  void initMorphoWordLists(string filename);

  /// @brief Get the morphology for a particular form
  ///
  /// @param form_id Word form id
  /// @param configuration Pointer to the instance of @ref Configuration class
  /// @return Factor list
  vector<FactorList> GetMorphology(unsigned form_id, Configuration* configuration);

  /// @brief Print out morphology
  ///
  /// @param ofs Output stream
  /// @param configuration Pointer to the instance of @ref Configuration class
  void PrintOut(ostream &ofs, Configuration* configuration);

  /// @brief Morphology constructor initialization. Reads morph data from file.
  ///
  /// @param ifs Input stream
  Morphology(ifstream &ifs);

  /// @brief Write the morphology to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs);
};

SP_DEF(Morphology);

} // namespace korektor
} // namespace ufal
