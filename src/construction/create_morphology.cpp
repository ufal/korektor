// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <tuple>

#include "common.h"
#include "lexicon/lexicon.h"
#include "morphology/morphology.h"
#include "persistent_structures/bit_array.h"
#include "spellchecker/configuration.h"
#include "utils/utils.h"

#define MAX_GROUP_COUNT 100000

using namespace ufal::korektor;

unsigned node_id_counter;

class morpho_node;
SP_DEF(morpho_node);

class morpho_node {
 public:
  unsigned factor_id;
  unsigned node_id;
  unsigned num_children;
  //map<unsigned, morpho_nodeP > children;
  map<unsigned, morpho_nodeP> children;

  morpho_node(unsigned _factor_id): factor_id(_factor_id), num_children(0)
  {
    node_id = node_id_counter;
    node_id_counter++;
  }
};


const unsigned MINIMAL_COUNT_FOR_ADDING_NEW_WORD = 200;

struct Emissions {

  //map<pair<unsigned, pair<unsigned, unsigned> >, unsigned> jointCounts;
  //map<> counts;

  map<tuple<unsigned, unsigned, unsigned>, unsigned> jointCounts;
  map<pair<unsigned, unsigned>, unsigned> counts;

  ValueMappingP value_mapping;

 public:
  void AddCount(unsigned form_id, unsigned factor_index, unsigned factor_id, unsigned count = 1)
  {
    tuple<unsigned, unsigned, unsigned> key = make_tuple(factor_index, factor_id, form_id);

    if (jointCounts.find(key) == jointCounts.end())
    {
      jointCounts[key] = count;
    }
    else
    {
      jointCounts[key] += count;
    }

    pair<unsigned, unsigned> key2 = make_pair(factor_index, factor_id);

    if (counts.find(key2) == counts.end())
    {
      counts[key2] = count;
    }
    else
    {
      counts[key2] += count;
    }

  }

  double GetEmissionCost(unsigned form_id, int factor_index, unsigned factor_id)
  {
    tuple<unsigned, unsigned, unsigned> key_joint = make_tuple(factor_index, factor_id, form_id);
    pair<unsigned, unsigned> key = make_pair(factor_index, factor_id);

    return (-log((double)jointCounts[key_joint] / (double)counts[key])) / log(10.0);
  }

  pair<unsigned, unsigned> GetEmissionCostMapped(unsigned form_id, int factor_index, unsigned factor_id)
  {
    return make_pair(value_mapping->GetCenterID(GetEmissionCost(form_id, factor_index, factor_id)), value_mapping->BitsPerValue());
  }

  void init_value_mapping()
  {
    vector<double> costs;
    for (map<tuple<unsigned, unsigned, unsigned>, unsigned>::iterator it = jointCounts.begin(); it != jointCounts.end(); it++)
    {
      tuple<unsigned, unsigned, unsigned> key = it->first;
      costs.push_back(GetEmissionCost(get<2>(key), get<0>(key), get<1>(key)));
    }

    value_mapping = ValueMappingP(new ValueMapping(costs, 8));
  }

};

class GroupFactors {
 private:
  vector<unsigned> factor_ids;
 public:

  GroupFactors(vector<unsigned> &_factor_ids)
  {
    factor_ids = _factor_ids;
    std::sort(factor_ids.begin(), factor_ids.end());
  }

  unsigned NumFactors()
  {
    return factor_ids.size();
  }

  unsigned GetFactorAt(unsigned index)
  {
    return factor_ids[index];
  }
};

SP_DEF(GroupFactors);

struct GroupFactorsP_iequal_to: std::binary_function<GroupFactorsP, GroupFactorsP, bool>
{
  bool operator()(GroupFactorsP const& x, GroupFactorsP const& y) const
  {
    if (x->NumFactors() != y->NumFactors())
      return false;


    for (uint32_t i = 0; i < x->NumFactors(); i++)
    {
      if (x->GetFactorAt(i) != y->GetFactorAt(i))
        return false;
    }

    return true;
  }
};


struct GroupFactorsP_ihash: std::unary_function<GroupFactorsP, std::size_t>
{
  std::size_t operator()(GroupFactorsP const& x) const
  {
    std::size_t seed = 0;

    Utils::HashCombine(seed, x->NumFactors());

    for (uint32_t i = 0; i < x->NumFactors(); i++)
    {
      Utils::HashCombine(seed, x->GetFactorAt(i));
    }

    return seed;
  }
};

typedef unordered_map<GroupFactorsP, unsigned, GroupFactorsP_ihash, GroupFactorsP_iequal_to> map_GroupFactorsP;

struct Dependency {
  unsigned governing_factor;
  unsigned governed_factor;

  map<unsigned, unsigned> dependency_map;

  Dependency(unsigned _governing_factor, unsigned _governed_factor, map<unsigned, unsigned> &_dependency_map):
    governing_factor(_governing_factor), governed_factor(_governed_factor), dependency_map(_dependency_map) {}
};

struct CM_variables {
  vector<string> factors; //name of the factors;
  morpho_nodeP root; //root of the morphology tree;
  vector<vector<morpho_nodeP> > levels; //level sets of the morphology tree;
  vector<unsigned> bits_per_value; //number of bits needed to store a factorID for the particular factor
  vector<unsigned> bits_per_children; //number of bits needed to store a number of children for a node at the particular level or a groupID if the level is grouped
  vector<bool> is_factor_grouped; //true if the factors at the given levels are grouped;
  vector<bool> is_factor_dependant;
  vector<vector<string> > factor_string_lists;
  vector<uint32_t> factor_next_id;
  vector<map<string, uint32_t> > factor_dictionaries;
  vector<Dependency> dependencies; //list of all factor dependencies
  map<uint32_t, vector<GroupFactorsP> > groupings;
  vector<pair<unsigned, unsigned> > bit_array_data;
  map<unsigned, unsigned> node_group_map;
  unsigned num_factors;
  map<pair<unsigned, unsigned>, bool> is_dependence;
  map<pair<unsigned, unsigned>, map<unsigned, unsigned> > dep_struct;
  Emissions emissions;

  void init(unsigned _num_factors)
  {
    if (_num_factors > FactorList::MAX_FACTORS)
      runtime_errorf("Too many morphological factors: '%u'. The limit is '%u'!", _num_factors, unsigned(FactorList::MAX_FACTORS));

    num_factors = _num_factors;

    for (size_t i = 0; i < num_factors; i++)
    {
      levels.push_back(vector<morpho_nodeP>());
      bits_per_value.push_back(0);

      if (i < num_factors - 1)
        bits_per_children.push_back(0);

      is_factor_grouped.push_back(true);
      is_factor_dependant.push_back(true);
      factor_string_lists.push_back(vector<string>());
      factor_next_id.push_back(0);
      factor_dictionaries.push_back(map<string, uint32_t>());
    }


    for (uint32_t i = 0; i < num_factors - 1; i++)
      for (uint32_t j = i + 1; j < num_factors; j++)
      {
        is_dependence[make_pair(i, j)] = true;
        dep_struct[make_pair(i,j)] = map<unsigned, unsigned>();
      }

    is_factor_grouped[0] = false;
    is_factor_dependant[0] = false;

    root = morpho_nodeP(new morpho_node(0));
  }
};

CM_variables cm;


void resolve_dependencies_traverse_nodes(vector<morpho_nodeP> &chain)
{
  morpho_nodeP top = chain[chain.size() - 1];

  unsigned top_level = chain.size() - 1;

  for (unsigned i = 0; i < top_level; i++)
  {
    if (cm.is_dependence[make_pair(i, top_level)])
    {
      unsigned factorID = chain[i]->factor_id;
      pair<unsigned, unsigned> key1 = make_pair(i, top_level);

      if (cm.dep_struct[key1].find(factorID) == cm.dep_struct[key1].end())
      {
        cm.dep_struct[key1][factorID] = top->factor_id;
      }
      else
      {
        if (cm.dep_struct[key1][factorID] != top->factor_id)
        {
          cm.is_dependence[make_pair(i, top_level)] = false;
        }
      }
    }
  }

  for (auto it = top->children.begin(); it != top->children.end(); it++)
  {
    chain.push_back(it->second);
    resolve_dependencies_traverse_nodes(chain);
    chain.pop_back();
  }
}

void makeGrouping()
{
  for (uint32_t i = 0; i < cm.num_factors - 1; i++)
  {
    unsigned nongrouped_bits_needed = cm.levels[i].size() * cm.bits_per_children[i] + cm.levels[i + 1].size() * cm.bits_per_value[i + 1];

    map_GroupFactorsP group_map;
    vector<GroupFactorsP> group_vector;
    unsigned num_group_elements = 0;

    cerr << "make grouping: level = " << i << endl;

    for (size_t j = 0; j < cm.levels[i].size(); j++)
    {
      //if (j % 1000 == 0) cerr << "make grouping: level = " << i << ", node = " << j << endl;

      morpho_nodeP node = cm.levels[i][j];

      vector<unsigned> gr_factors_vec;

      for (auto it = node->children.begin(); it != node->children.end(); it++)
      {
        gr_factors_vec.push_back(it->first);
      }

      GroupFactorsP gf = GroupFactorsP(new GroupFactors(gr_factors_vec));

      if (group_map.find(gf) == group_map.end())
      {
        group_map[gf] = group_vector.size();
        group_vector.push_back(gf);
        num_group_elements += gf->NumFactors();

        if (group_vector.size() % 10 == 0)
        {
          unsigned grouped_bits_needed = cm.levels[i].size() * Utils::BitsNeeded(group_vector.size() - 1) + num_group_elements * cm.bits_per_value[i + 1] + group_vector.size() * 16;

          if (grouped_bits_needed * 2 > nongrouped_bits_needed)
          {
            cerr << "grouping refused for factor " << (i + 1) << ", number of groups so far: " << group_vector.size() <<
                ", nongrouped_bits_needed: " << nongrouped_bits_needed << ", grouped_bits_needed: " << grouped_bits_needed << endl;
            cm.is_factor_grouped[i + 1] = false;
            break;
          }
        }
      }

      cm.node_group_map[node->node_id] = group_map[gf];
    }

    if (cm.is_factor_grouped[i + 1] == true)
    {
      cm.groupings[i + 1] = group_vector;
      cm.bits_per_children[i] = Utils::BitsNeeded(group_vector.size() - 1);
      cm.is_factor_dependant[i + 1] = false;
      for (unsigned j = 0; j < i + 1; j++)
        cm.is_dependence[make_pair(j, i + 1)] = false;
    }
  }
}

void resolve_dependencies()
{

  for (size_t i = 0 ; i < cm.levels[0].size(); i++)
  {
    morpho_nodeP node = cm.levels[0][i];
    vector<morpho_nodeP> chain;
    chain.push_back(node);

    if (i % 400000 == 0) cerr << "resolving dependencies, level[0] node = " << i << endl;

    resolve_dependencies_traverse_nodes(chain);
  }

  for (unsigned i = 1; i < cm.levels.size(); i++)
  {
    bool dependant = false;
    unsigned best_governing_factor = 0; // To keep compiler happy
    unsigned best_governing_factor_vocab_size = 0; // To keep compiler happy

    for (unsigned j = 0; j < i; j++)
    {
      if (cm.is_dependence[make_pair(j, i)])
      {
        cerr << "dependency detected: " << j << " --> " << i << endl;
        if (dependant == false || best_governing_factor_vocab_size > cm.levels[j].size())
        {
          dependant = true;
          best_governing_factor = j;
          best_governing_factor_vocab_size = cm.levels[j].size();
        }
      }
    }

    cm.is_factor_dependant[i] = dependant;

    if (dependant == true)
    {
      cm.dependencies.push_back(Dependency(best_governing_factor, i, cm.dep_struct[make_pair(best_governing_factor, i)]));
    }
  }
}


void save_node(morpho_nodeP node, vector<pair<uint32_t, unsigned> > &values, unsigned _level, unsigned formID, bool save_factor_id, uint32_t &bits_used)
{
  if (_level > 0)
  {
    pair<unsigned, unsigned> cost_pair = cm.emissions.GetEmissionCostMapped(formID, _level, node->factor_id);
    values.push_back(cost_pair);
    bits_used += cost_pair.second;
  }

  if (_level > 0 && save_factor_id && !cm.is_factor_dependant[_level])
  {
    values.push_back(make_pair(node->factor_id, cm.bits_per_value[_level]));
    bits_used += cm.bits_per_value[_level];
  }

  if (_level < cm.levels.size() - 1)
  {
    if (cm.is_factor_dependant[_level + 1] == false)
    {
      if (cm.is_factor_grouped[_level + 1])
        values.push_back(make_pair(cm.node_group_map[node->node_id], cm.bits_per_children[_level]));
      else
      {
        assert(node->num_children > 0);
        values.push_back(make_pair(node->num_children - 1, cm.bits_per_children[_level]));
      }
    }

    bits_used += cm.bits_per_children[_level];

    for (auto it = node->children.begin(); it != node->children.end(); it++)
    {
      if (cm.is_factor_grouped[_level + 1])
        save_node(it->second, values, _level + 1, formID, false, bits_used);
      else
        save_node(it->second, values, _level + 1, formID, true, bits_used);
    }
  }

}

void construct_levels(morpho_nodeP node, unsigned act_level)
{
  cm.levels[act_level].push_back(node);

  for (auto it = node->children.begin(); it != node->children.end(); it++)
  {
    morpho_nodeP child_node = it->second;
    construct_levels(child_node, act_level + 1);
  }
}

bool MorphologyProcessWordFactors(const string &factor_string, bool possibly_add_zero_level_node, bool add_counts_for_nodes_just_being_visited, unsigned count = 1)
{
  vector<string> parts;
  Utils::Split(parts, factor_string, "|");

  morpho_nodeP node = cm.root;

  unsigned form_id;

  for (unsigned i = 0; i < parts.size(); i++)
  {
    if (cm.factor_dictionaries[i].find(parts[i]) == cm.factor_dictionaries[i].end())
    {
      if (i > 0 || possibly_add_zero_level_node == true || count >= MINIMAL_COUNT_FOR_ADDING_NEW_WORD)
      {
        cm.factor_dictionaries[i][parts[i]] = cm.factor_next_id[i];
        cm.factor_string_lists[i].push_back(parts[i]);
        cm.factor_next_id[i]++;
      }
      else
      {
        return false;
      }
    }

    unsigned factorID = cm.factor_dictionaries[i][parts[i]];

    if (i == 0) form_id = factorID;


    if (node->children.find(factorID) == node->children.end())
    {
      node->children[factorID] = morpho_nodeP(new morpho_node(factorID));
      node->num_children++;

      if (i > 0)
        cm.emissions.AddCount(form_id, i, factorID, count);
    }
    else
    {
      if (i > 0 && add_counts_for_nodes_just_being_visited)
        cm.emissions.AddCount(form_id, i, factorID, count);
    }

    node = node->children[factorID];
  }

  return true;
}

string createSpecialMorphologyEntry(const string &spec)
{
  stringstream strs;
  for (unsigned i = 0; i < cm.num_factors; i++)
  {
    if (i > 0)
      strs << "|";
    strs << spec;
  }

  return strs.str();
}

int main(int argc, char** argv)
{
  node_id_counter = 0;

  ifstream ifs;

  if (argc != 6)
  {
    cerr << "morphology directory expected!";
    exit(1);
  }

  string morpho_file = argv[1];
  string emissions_file = argv[2];
  string output_file = argv[3];
  string output_vocab_file = argv[4];
  string test_file = argv[5];


  ifs.open(morpho_file.c_str(), ios::in);
  if (!ifs.is_open())
  {
    cerr << "Can't open morphology file!";
    exit(1);
  }

  string s;
  Utils::SafeReadline(ifs, s);
  vector<string> parts;

  Utils::Split(parts, s, "|");

  cm.init(parts.size());

  for (unsigned i = 0; i < parts.size(); i++)
  {
    cm.factors.push_back(parts[i]);
  }
  cm.num_factors = cm.factors.size();

  MorphologyProcessWordFactors(createSpecialMorphologyEntry("<s>"), true, true);
  MorphologyProcessWordFactors(createSpecialMorphologyEntry("</s>"), true, true);
  MorphologyProcessWordFactors(createSpecialMorphologyEntry("<number>"), true, true);
  MorphologyProcessWordFactors(createSpecialMorphologyEntry("<name>"), true, true);
  MorphologyProcessWordFactors(createSpecialMorphologyEntry("<unk>"), true, true);

  Utils::SafeReadline(ifs, s);

  if (s != "-----")
  {
    cerr << "the second line of the morphology file should be: -----" << endl;
    return -1;
  }

  vector<string> orig_lines;
  unsigned counter = 0;
  while (Utils::SafeReadline(ifs, s))
  {
    counter++;
    if (counter % 10000 == 0) { cerr << "morphology_reading: " << counter << endl; }
    //if (s != "")
    //  orig_lines.push_back(s);
    MorphologyProcessWordFactors(s, true, false);
  }

  cerr << "morphology input file processed\n";

  ifs.close();

  ifs.open(emissions_file.c_str(), ios::in);

  if (ifs.is_open() == false)
  {
    cerr << "Can't open " << emissions_file << std::endl;
    return -1;
  }

  counter = 0;
  string factors;
  unsigned count;

  while (Utils::SafeReadline(ifs, s))
  {
    counter++;
    if (counter % 10000 == 0) { cerr << "reading_corpora: " << counter << endl; }
    Utils::Split(parts, s, " ");
    factors = parts[0];

    if (parts.size() < 2)
    {
      cerr << "Illegal emissions line: " << s << endl;
    }

    count = Utils::my_atoi(parts[1]);

    //passing 10 x count instead of just count reduces discounting (non-seen form-factor pairs have effectively just count 0.1
    MorphologyProcessWordFactors(factors, false, true, count * 10);

  }

  ifs.close();

  cerr << "corpus data readed\n";

  for (auto it = cm.root->children.begin(); it != cm.root->children.end(); it++)
  {
    construct_levels(it->second, 0);
  }

  cerr << "level sets constructed\n";



  for (unsigned i = 0; i < cm.levels.size() - 1; i++)
  {
    cerr << "estimating max children on level " << i << endl;
    unsigned max_children = 0;

    for (unsigned j = 0; j < cm.levels[i].size(); j++)
    {
      if (j % 1000 == 0) cerr << j << endl;
      morpho_nodeP node = cm.levels[i][j];

      if (node->num_children > 0 && node->num_children - 1 > max_children)
        max_children = node->num_children - 1;
    }

    cm.bits_per_children[i] = Utils::BitsNeeded(max_children);
  }

  for (unsigned i = 0; i < cm.levels.size(); i++)
  {
    cerr << "estimating max value on level " << i << endl;
    unsigned max_value = 0;

    for (unsigned j = 0; j < cm.levels[i].size(); j++)
    {
      morpho_nodeP node = cm.levels[i][j];

      if (node->factor_id > max_value)
        max_value = node->factor_id;
    }

    cm.bits_per_value[i] = Utils::BitsNeeded(max_value);
  }

  cerr << "grouping..." << endl;
  makeGrouping();
  cerr << "grouping detection finished\n";


  resolve_dependencies();

  ofstream ofs;

  ofs.open(output_file.c_str(), ios::out | ios::binary);
  if (!ofs.is_open())
  {
    cerr << "can't create morphology.bin output file!\n";
    exit(1);
  }

  Utils::WriteString(ofs, "Morphology");

  ofs.write((char*)&cm.num_factors, sizeof(uint32_t));
  cerr << "writting num factors: " << cm.num_factors << endl;

  StringArray mssa = StringArray(cm.factors);
  mssa.WriteToStream(ofs);

  uint32_t dep_size = cm.dependencies.size();
  ofs.write((char*)&dep_size, sizeof(uint32_t));

  for (unsigned i = 0; i < dep_size; i++)
  {
    uint32_t governing = cm.dependencies[i].governing_factor;
    uint32_t governed = cm.dependencies[i].governed_factor;
    ofs.write((char*)&governing, sizeof(uint32_t));
    ofs.write((char*)&governed, sizeof(uint32_t));

    map<unsigned, unsigned> dep_map = cm.dependencies[i].dependency_map;
    vector<uint32_t> dep_vector;

    for (unsigned j = 0; j < cm.factor_string_lists[governing].size(); j++)
      dep_vector.push_back(std::numeric_limits<uint32_t>::max());

    for (map<unsigned, unsigned>::iterator it = dep_map.begin(); it != dep_map.end(); it++)
    {
      dep_vector[it->first] = it->second;
    }

    for (unsigned i = 0; i < dep_vector.size(); i++)
    {
      assert(dep_vector[i] != std::numeric_limits<uint32_t>::max());
    }

    PackedArray mpa = PackedArray(dep_vector);
    mpa.WriteToStream(ofs);
  }

  unsigned num_grouped = 0;
  for (unsigned i = 0; i < cm.num_factors; i++)
  {
    if (cm.is_factor_grouped[i])
      num_grouped++;
  }

  ofs.write((char*)&num_grouped, sizeof(uint32_t));

  for (map<uint32_t, vector<GroupFactorsP> >::iterator it = cm.groupings.begin(); it != cm.groupings.end(); it++)
  {
    unsigned factor_index = it->first;
    ofs.write((char*)&factor_index, sizeof(uint32_t));

    vector<uint32_t> group_values;
    vector<uint32_t> group_pointers;

    vector<GroupFactorsP> vec_gf = it->second;

    for (unsigned i = 0; i < vec_gf.size(); i++)
    {
      group_pointers.push_back(group_values.size());
      for (unsigned j = 0; j < vec_gf[i]->NumFactors(); j++)
      {
        group_values.push_back(vec_gf[i]->GetFactorAt(j));
      }
    }

    CompIncreasingArray cia_ble = CompIncreasingArray(group_pointers, group_values.size() - 1);
    PackedArray mpa_ble = PackedArray(group_values);

    cia_ble.WriteToStream(ofs);
    mpa_ble.WriteToStream(ofs);
  }

  for (unsigned i = 0; i < cm.bits_per_value.size(); i++)
  {
    ofs.write((char*)&(cm.bits_per_value[i]), sizeof(uint32_t));
  }

  for (unsigned i = 0; i < cm.bits_per_children.size(); i++)
  {
    ofs.write((char*)&(cm.bits_per_children[i]), sizeof(uint32_t));
  }

  cm.emissions.init_value_mapping();
  cm.emissions.value_mapping->writeToStream(ofs);

  vector<pair<uint32_t, unsigned> > values;

  unsigned first_level_num_nodes = cm.levels[0].size();
  //ofs.write((char*)&first_level_num_nodes, sizeof(uint32_t));

  cerr << "first level num nodes = " << first_level_num_nodes << endl;

  uint32_t bits_used = 0;
  vector<uint32_t> form_offsets;
  for (unsigned i = 0; i < first_level_num_nodes; i++)
  {
    form_offsets.push_back(bits_used);
    save_node(cm.levels[0][i], values, 0, cm.levels[0][i]->factor_id, true, bits_used);
  }

  CompIncreasingArray mia_offsets = CompIncreasingArray(form_offsets, bits_used - 1);
  mia_offsets.WriteToStream(ofs);

  BitArray mba_values = BitArray(values);
  mba_values.WriteToStream(ofs);

  Lexicon lexicon = Lexicon::fromUTF8Strings(cm.factor_string_lists[0]);

  lexicon.WriteToStream(ofs);
  //lexicon.PrintWords(cout);
  ofs.close();

  ofs.open(output_vocab_file.c_str(), ios::out | ios::binary);
  assert(ofs.is_open());

  ofs.write((char*)&(cm.num_factors), sizeof(uint32_t));

  for (unsigned i = 0; i < cm.num_factors; i++)
  {
    StringArray mssa = StringArray(cm.factor_string_lists[i]);
    mssa.WriteToStream(ofs);
  }

  ofs.close();

  string::iterator it;



  ofstream testout;
  testout.open(test_file.c_str(), ios::out);
  assert(testout.is_open());

  ifs.open(output_file.c_str(), ios::in | ios::binary);
  if (!ifs.is_open())
    runtime_errorf("Cannot open file '%s'!", output_file.c_str());
  MorphologyP morphology = MorphologyP(new Morphology(ifs));
  morphology->initMorphoWordLists(output_vocab_file);

  LexiconP lex2 = LexiconP(new Lexicon(ifs));
  testout << "LEXICON\n=======================================================================\n";
  lex2->PrintWords(testout);

  ifs.close();

  ConfigurationP configuration = ConfigurationP(new Configuration());
  configuration->LoadMorphologyAndLexicon(lex2, morphology);

  for (auto it = cm.factors.begin(); it != cm.factors.end(); it++)
  {
    if (Utils::randomR(0, 2) == 0)
      configuration->EnableFactor(*it, 0.5, 3);
  }

//  configuration->EnableFactor("lemma", 0.15, 3);
//  configuration->EnableFactor("form", 0.5, 3);
//  configuration->EnableFactor("tag", 0.35, 3);
//  configuration->EnableFactor("short", 0.1, 3);

  testout << "MORPHOLOGY\n=======================================================================\n";
  morphology->PrintOut(testout, configuration.get());

  testout.close();

//  ifstream ifstest;
//  vector<string> test_lines;
//
//  ifstest.open(test_file.c_str(), ios::in);
//  assert(ifstest.is_open());
//
//  while (Utils::SafeReadline(ifstest, s))
//  {
//    if (s != "")
//      test_lines.push_back(s);
//  }
//
//  cerr << "finishing test, sorting..." << endl;
//  std::sort(test_lines.begin(), test_lines.end());
//  std::sort(orig_lines.begin(), orig_lines.end());
//
//  cerr << "checking consistency..." << endl;
//  cerr << "test_lines: " << test_lines.size() << endl;
//  cerr << "orig_lines: " << orig_lines.size() << endl;
//
//  for (unsigned i = 0; i < test_lines.size(); i++)
//  {
//    cout << i << endl;
//    //if (test_lines[i] != orig_lines[i])
//    cout << test_lines[i] << " --- " << orig_lines[i] << endl;
//  }

  cerr << "OK!\n";
  exit(0);
  return 0;
}
