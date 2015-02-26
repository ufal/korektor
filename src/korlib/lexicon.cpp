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

#include "lexicon.h"
#include "utils.h"
#include "my_packed_array.h"
#include "comp_increasing_array.h"
#include "error_model.h"

namespace ufal {
namespace korektor {

unsigned lexicon_node::num_nodes = 0;
map<unsigned, lexicon_nodeP> lexicon_node::nodes_map = map<unsigned, lexicon_nodeP>();

/// @brief Initialize lexicon from another lexicon
///
/// @param val Lexicon
Lexicon::Lexicon(const Lexicon& val)
{
  num_arcs = val.num_arcs;

  arcs_char = new char16_t[num_arcs];
  memcpy(arcs_char, val.arcs_char, num_arcs * sizeof(char16_t));

  arcs_nextstate = new uint32_t[num_arcs];
  memcpy(arcs_nextstate, val.arcs_nextstate, num_arcs * sizeof(uint32_t));

  num_words = val.num_words;
  states_arcpointer = val.states_arcpointer;
  root_id = val.root_id;
}

/// @brief Copy constructor
///
/// @param val Lexicon
/// @return Reference to created lexicon
Lexicon& Lexicon::operator=(const Lexicon& val)
{
  if (this != &val)
  {
    num_arcs = val.num_arcs;
    char16_t* new_arcs_char = new char16_t[num_arcs];
    memcpy(new_arcs_char, val.arcs_char, num_arcs * sizeof(char16_t));

    num_words = val.num_words;

    uint32_t *new_arcs_nextstate = new uint32_t[num_arcs];
    memcpy(new_arcs_nextstate, val.arcs_nextstate, num_arcs * sizeof(uint32_t));

    states_arcpointer = val.states_arcpointer;

    delete[] arcs_char;
    delete[] arcs_nextstate;

    arcs_char = new_arcs_char;
    arcs_nextstate = new_arcs_nextstate;

    root_id = val.root_id;
  }

  return *this;
}

/// @brief Get the number of states
///
/// @return Number of states (integer)
uint32_t Lexicon::NumStates() const { return states_arcpointer.GetSize(); }


int Lexicon::ContainsSuffix(uint32_t stateID, const u16string &word, uint32_t start_char_index) const
{
  int sID = (int)stateID;
  for (uint32_t i = start_char_index; i < (unsigned)word.length(); i++)
  {

    sID = SingleArc_nextstate((uint32_t)sID, word[i]);
    if (sID == -1)
      return -1;
  }

  if (StateIsWord((uint32_t)sID))
    return sID;// GetWordID_per_state_id((uint32_t)sID);
  else
    return -1;
}


/// @brief Check whether the node of the given node ID is a dictionary word
///
/// @param state_ID Node ID
/// @return "true" if the node is a word "false" otherwise
bool Lexicon::StateIsWord(uint32_t state_ID) const
{
  return state_ID < root_id;
}

int Lexicon::SingleArc_nextstate(uint32_t stateID, char16_t character) const
{
  CompIA_First_Last_IndexPair index_pair = states_arcpointer.GetFirstLastIndexPair(stateID);

  //if (index_pair == NULL)
  //  return -1;

  if (index_pair.first > index_pair.second)
    return -1;

  uint32_t leftIndex = index_pair.first;
  uint32_t rightIndex = index_pair.second;
  uint32_t actIndex;

  while (true)
  {
    actIndex = (leftIndex + rightIndex) >> 1;
    char16_t arc_char = arcs_char[actIndex];

    if (character == arc_char )
    {
      return arcs_nextstate[actIndex];
    }

    if (arc_char > character)
    {
      if (rightIndex == actIndex)
        return -1;
      rightIndex = actIndex;
    }
    else
    {

      if (leftIndex == actIndex && rightIndex == leftIndex + 1)
      {
        leftIndex = rightIndex;
        continue;
      }

      leftIndex = actIndex;
    }

    if (leftIndex == rightIndex)
      break;

  }

  return -1;
}

/// @brief Constructor initialization from an input stream
///
/// @param ifs Input stream
Lexicon::Lexicon(istream &ifs)
{
  string checkIT = MyUtils::ReadString(ifs);

  FATAL_CONDITION(checkIT == "Lexicon", "checkIT = " << checkIT);

  ifs.read((char*)&num_words, sizeof(uint32_t));
  ifs.read((char*)&num_arcs, sizeof(uint32_t));
  ifs.read((char*)&root_id, sizeof(uint32_t));


  states_arcpointer = CompIncreasingArray(ifs);

  arcs_nextstate = new uint32_t[num_arcs];
  ifs.read((char*)arcs_nextstate, num_arcs * sizeof(uint32_t));

  arcs_char = new char16_t[num_arcs];
  ifs.read((char*)arcs_char, num_arcs * sizeof(char16_t));

  uint32_t num_noncorrectable;
  ifs.read((char*)&num_noncorrectable, sizeof(uint32_t));

  for (unsigned i = 0; i < num_noncorrectable; i++)
  {
    uint32_t wordID;
    ifs.read((char*)&wordID, sizeof(uint32_t));
    noncorrectable_word_ids.insert(wordID);
  }

}

/// @brief Get the word ID
///
/// @return Word ID (integer)
int Lexicon::GetWordID(const u16string &word_iso) const //return value -1 for out of vocabulary words!
{
  return ContainsSuffix(root_id, word_iso, 0);
}

/// @brief Create an entry for given word in the lexicon
///
/// @param node Current node
/// @param next_inner_node_id ID of the current node
/// @param curr_word Word to be added in the lexicon
/// @param char_index Current position within the word
/// @param words_map (word, ID) map
void Lexicon::create_lexicon_rec(lexicon_nodeP &node, unsigned &next_inner_node_id, const u16string &curr_word, unsigned char_index, map<u16string, unsigned> &words_map)
{
  char16_t ch = curr_word[char_index];
  map<char16_t, lexicon_nodeP>::iterator it1 = node->edges.find(ch);
  if (it1 == node->edges.end())
  {
    auto it2 = words_map.find(curr_word.substr(0, char_index + 1));
    if (it2 == words_map.end())
    {
      //prefix is not a dictionary entry - i.e. we assign an inner node id
      node->edges[ch] = lexicon_node::create_node(next_inner_node_id);
      next_inner_node_id++;

      if (char_index + 1 < (unsigned)curr_word.length())
        create_lexicon_rec(node->edges[ch], next_inner_node_id, curr_word, char_index + 1, words_map);
    }
    else
    {
      //prefix is a dictionary entry - outer node id is assigned
      node->edges[ch] = lexicon_node::create_node(it2->second);

      if (char_index + 1 < (unsigned)curr_word.length())
        create_lexicon_rec(node->edges[ch], next_inner_node_id, curr_word, char_index + 1, words_map);
    }
  }
  else
  {
    if (char_index + 1 < (unsigned)curr_word.length())
      create_lexicon_rec(it1->second, next_inner_node_id, curr_word, char_index + 1, words_map);
  }
}

/// @brief Initialize lexicon from the vector of words. The strings must be in UTF-16 format.
///
/// @param words Vector of words
Lexicon::Lexicon(const vector<u16string> &words)
{
  lexicon_node::num_nodes = 0;
  lexicon_node::nodes_map.clear();

  num_words = words.size();
  unsigned next_inner_node_id = num_words;

  lexicon_nodeP root = lexicon_node::create_node(next_inner_node_id);
  next_inner_node_id++;

  map<u16string, unsigned> words_map;
  for (unsigned i = 0; i < words.size(); i++)
    words_map[words[i]] = i;

  for (unsigned i = 0; i < num_words; i++)
  {
    create_lexicon_rec(root, next_inner_node_id, words[i], 0, words_map);
  }

  unsigned curr_offset = 0;
  vector<char16_t> edges_char;
  vector<uint32_t> edges_endpoint;
  vector<uint32_t> offsets;

  for (unsigned i = 0; i < lexicon_node::num_nodes; i++)
  {
    lexicon_nodeP node = lexicon_node::nodes_map[i];

    offsets.push_back(curr_offset);

    for (map<char16_t, lexicon_nodeP>::iterator it = node->edges.begin(); it != node->edges.end(); it++)
    {
      edges_char.push_back(it->first);
      edges_endpoint.push_back(it->second->node_id);
      curr_offset++;
    }
  }

  root_id = root->node_id;
  states_arcpointer = CompIncreasingArray(offsets, curr_offset - 1);
  num_arcs = edges_endpoint.size();

  arcs_nextstate = new uint32_t[num_arcs];
  memcpy(arcs_nextstate, edges_endpoint.data(), num_arcs * sizeof(uint32_t));

  arcs_char = new char16_t[num_arcs];
  memcpy(arcs_char, edges_char.data(), num_arcs * sizeof(char16_t));
}

/// @brief Initialize lexicon from a vector of words
///
/// @param words Vector of words
/// @return Lexicon created from the given vector or words
Lexicon Lexicon::fromUTF8Strings(const vector<string> &words)
{
  vector<u16string> uc_words;
  uc_words.reserve(uc_words.size());

  for (auto it = words.begin(); it != words.end(); it++)
  {
    uc_words.push_back(MyUtils::utf8_to_utf16(*it));
  }

  return Lexicon(uc_words);
}

/// @brief Write lexicon to output stream
///
/// @param ofs Output stream
void Lexicon::WriteToStream(ostream &ofs) const
{
  MyUtils::WriteString(ofs, "Lexicon");
  ofs.write((char*)&num_words, sizeof(uint32_t));
  ofs.write((char*)&num_arcs, sizeof(uint32_t));
  ofs.write((char*)&root_id, sizeof(uint32_t));

  states_arcpointer.WriteToStream(ofs);

  ofs.write((char*)arcs_nextstate, num_arcs * sizeof(uint32_t));

  ofs.write((char*)arcs_char, num_arcs * sizeof(char16_t));

  unsigned num_noncorrectable = noncorrectable_word_ids.size();
  ofs.write((char*)&num_noncorrectable, sizeof(uint32_t));

  for (auto it = noncorrectable_word_ids.begin(); it != noncorrectable_word_ids.end(); it++)
  {
    ofs.write((char*)&(*it), sizeof(uint32_t));
  }
}

void Lexicon::ArcsConsistencyCheck()
{
  unordered_set<uint32_t> arcs_next_used;
  cerr << "num_arcs = " << num_arcs << endl;
  for (unsigned i = 0; i < num_arcs; i++)
  {
    if (i % 1000 == 0) cerr << i << endl;
    uint32_t arc_next = arcs_nextstate[i];
    FATAL_CONDITION(arcs_next_used.find(arc_next) == arcs_next_used.end(), "blebleble! i = " << i << ", arc_next = " << arc_next);
    arcs_next_used.insert(arc_next);
  }

}

void Lexicon::print_words_rec(unsigned node_id, u16string &prefix, map<unsigned, u16string> &words, unsigned &index, uint32_t max_index)
{
  if (index >= max_index)
    return;

  if (node_id < num_words)
  {
    words[node_id] = prefix;
    index++;
  }

  if (index >= max_index)
    return;

  CompIA_First_Last_IndexPair ipair = states_arcpointer.GetFirstLastIndexPair(node_id);

  for (unsigned i = ipair.first; i <= ipair.second; i++)
  {
    //prefix.push_back((*arcs)[i].character);
    prefix += arcs_char[i];
    print_words_rec(arcs_nextstate[i], prefix, words, index, max_index);
    prefix.erase(prefix.length() - 1, 1);
    if (index >= max_index) return;
    //prefix.pop_back();

  }

}

void Lexicon::PrintWords(ostream &os, uint32_t max_index)
{
  unsigned index = 0;
  map<unsigned, u16string> words;
  u16string prefix = MyUtils::utf8_to_utf16("");
  print_words_rec(root_id, prefix, words, index, max_index);

  for (auto it = words.begin(); it != words.end(); it++)
  {
    os << it->first << " - " << MyUtils::utf16_to_utf8(it->second) << endl;
  }
}

void Lexicon::AddSimilarWordToMap(Similar_Words_Map &ret, uint32_t word_id, double cost, u16string &word, uint32_t word_include_letter_start_index, u16string &prefix) const
{
  //bool ret_val = false;
  Similar_Words_Map::iterator it = ret.find(word_id);
  if (it == ret.end())
  {
    u16string complete_word = prefix;
    for (uint32_t i = word_include_letter_start_index; i <(unsigned) word.length(); i++)
      complete_word += word[i];

    ret[word_id] = make_pair(complete_word, cost);
  }
  else if (it->second.second > cost)
  {
    ret[word_id].second = cost;
  }

  //return ret_val;
}


Similar_Words_Map Lexicon::GetSimilarWords_impl(u16string &word, uint32_t edit_distance, double cost, ErrorModelP errModel, uint32_t stateID, uint32_t startIndex, u16string &prefix, double cost_limit, bool ignore_case, uint32_t max_edit_distance)
{
  //cerr << edit_distance << endl;
  Similar_Words_Map ret;
  double new_cost;
  uint32_t new_edit_dist;
  ErrorModelOutput emo;

  uint32_t init_prefix_size = prefix.length();

  //cerr << "prefix = " << MyUtils::utf16_to_utf8(prefix) << endl;

  for (uint32_t i = startIndex; i <= (unsigned)word.length(); i++)
  {
    if (i > startIndex && i <= (unsigned)word.length()) //TODO: Check this
      prefix += word[i - 1];

    if (i == word.length() && StateIsWord(stateID))
    {
      AddSimilarWordToMap(ret, stateID, cost, word, i, prefix);
    }

    int next_state = -1;
    CompIA_First_Last_IndexPair ipair = states_arcpointer.GetFirstLastIndexPair(stateID);
    if (i < (unsigned)word.length())
    {

      if ((unsigned)word.length() > 1)
      {
//        if (i == 0)
//          emo = errModel->DeletionCost(word[i], char16_t(' '));
//        else
//          emo = errModel->DeletionCost(word[i], word[i - 1]);

        if (i == 0)
          emo = errModel->InsertionCost(word[i], char16_t(' '), word[i + 1]);
        else if (i < word.length() - 1)
          emo = errModel->InsertionCost(word[i], word[i - 1], word[i + 1]);
        else
          emo = errModel->InsertionCost(word[i], word[i - 1], char16_t(' '));

        new_cost = cost + emo.cost;
        new_edit_dist = edit_distance + emo.edit_dist;

        if (new_cost <= cost_limit && new_edit_dist <= max_edit_distance)
        {
          int wID = ContainsSuffix(stateID, word, i + 1);
          if (wID != -1)
            AddSimilarWordToMap(ret, wID, new_cost, word, i + 1, prefix);

          if (new_edit_dist + errModel->MinOperationEditDistance() <= max_edit_distance)
          {
            Similar_Words_Map sim_w = GetSimilarWords_impl(word, new_edit_dist, new_cost, errModel, stateID, i + 1, prefix, cost_limit, ignore_case, max_edit_distance);
            ret.insert(sim_w.begin(), sim_w.end());
          }

        }
      }

      for (uint32_t j = ipair.first; j <= ipair.second; j++) //letter substitution
      {
        u16string char_debug;
        char_debug.push_back(arcs_char[j]);
        char16_t character = arcs_char[j];
        unsigned arc_nextstate = arcs_nextstate[j];

        if (character != word[i])
        {
          emo = errModel->SubstitutionCost(word[i], character, ignore_case);
          new_cost = cost + emo.cost;
          new_edit_dist = edit_distance + emo.edit_dist;

          if (new_cost <= cost_limit && new_edit_dist <= max_edit_distance)
          {

            int wID = ContainsSuffix(arc_nextstate, word, i + 1);

            prefix += character;

            if (wID != -1)
              AddSimilarWordToMap(ret, wID, new_cost, word, i + 1, prefix);

            if (new_edit_dist + errModel->MinOperationEditDistance() <= max_edit_distance)
            {
              Similar_Words_Map sim_w = GetSimilarWords_impl(word, new_edit_dist, new_cost, errModel, arc_nextstate, i + 1, prefix, cost_limit, ignore_case, max_edit_distance);
              ret.insert(sim_w.begin(), sim_w.end());
            }

            prefix.erase(prefix.length() - 1, 1);
            //prefix.erase(prefix.size() - 1);
          }

        }
        else
        {
          next_state = (int)arc_nextstate;
        }
      }

      if (i < (unsigned)word.length() - 1) //letter swap
      {

        emo = errModel->SwapCost(word[i], word[i + 1]);
        new_cost = cost + emo.cost;
        new_edit_dist = edit_distance + emo.edit_dist;

        if (new_cost <= cost_limit && new_edit_dist <= max_edit_distance)
        {
          char16_t pom_swap = word[i];
          word[i] = word[i + 1];
          word[i + 1] = pom_swap;


          int wID = ContainsSuffix(stateID, word, i);
          if (wID != -1)
            AddSimilarWordToMap(ret, wID, new_cost, word, i, prefix);

          if (new_edit_dist + errModel->MinOperationEditDistance() <= max_edit_distance)
          {
            Similar_Words_Map sim_w = GetSimilarWords_impl(word, new_edit_dist, new_cost, errModel, stateID, i, prefix, cost_limit, ignore_case, max_edit_distance);
            ret.insert(sim_w.begin(), sim_w.end());
          }


          word[i + 1] = word[i];
          word[i] = pom_swap;
        }
      }

    } // if (i < word.size() )

    for (uint32_t j = ipair.first; j <= ipair.second; j++) //letter insertion
    {
      char16_t character = arcs_char[j];
      uint32_t arc_nextstate = arcs_nextstate[j];

//      if (i == 0)
//        emo = errModel->InsertionCost(character, char16_t(' '), word[0]);
//      else if (i == word.length())
//        emo = errModel->InsertionCost(character, word[i - 1], char16_t(' '));
//      else
//        emo = errModel->InsertionCost(character, word[i - 1], word[i]);

      if (i == 0)
        emo = errModel->DeletionCost(character, char16_t(' '));
      else
        emo = errModel->DeletionCost(character, char16_t(' '));

      new_cost = cost + emo.cost;
      new_edit_dist = edit_distance + emo.edit_dist;

      if (new_cost <= cost_limit && new_edit_dist <= max_edit_distance)
      {
        prefix += character;

        int wID = ContainsSuffix(arc_nextstate, word, i);
        if (wID != -1)
        {
          AddSimilarWordToMap(ret, wID, new_cost, word, i, prefix);
        }

        if (new_edit_dist + errModel->MinOperationEditDistance() <= max_edit_distance)
        {
          Similar_Words_Map sim_w = GetSimilarWords_impl(word, new_edit_dist, new_cost, errModel, arc_nextstate, i, prefix, cost_limit, ignore_case, max_edit_distance);
          ret.insert(sim_w.begin(), sim_w.end());
        }

        prefix.erase(prefix.length() - 1, 1);
        //prefix.erase(prefix.size() - 1);
      }

    }

    if (next_state == -1)
      break;
    stateID = next_state;

  }

  while ((unsigned)prefix.length() > init_prefix_size)
    prefix.erase(prefix.length() - 1, 1);

  return ret;
}

Similar_Words_Map Lexicon::GetSimilarWords(u16string word, uint32_t max_ed_dist, double _cost_limit, ErrorModelP errModel, bool _ignore_case)
{
  u16string prefix = MyUtils::utf8_to_utf16("");
  return GetSimilarWords_impl(word, 0, 0, errModel, root_id, 0, prefix, _cost_limit, _ignore_case, max_ed_dist);
}

vector<u16string> Lexicon::GetSimilarWords_string(u16string word, uint32_t max_ed_dist, double _cost_limit, ErrorModelP errModel, bool _ignore_case)
{
  vector<u16string> ret;

  Similar_Words_Map swm = GetSimilarWords(word, max_ed_dist, _cost_limit, errModel, _ignore_case);

  for (Similar_Words_Map::iterator it = swm.begin(); it != swm.end(); it++)
  {
    ret.push_back(it->second.first);
  }

  return ret;
}


void Lexicon::LoadListOfNoncorrectableWords(const vector<u16string> &noncorrectable_words)
{
  noncorrectable_word_ids.clear();

  for (auto it = noncorrectable_words.begin(); it != noncorrectable_words.end(); it++)
  {
    int wordID = GetWordID(*it);
    assert(wordID != -1);
    noncorrectable_word_ids.insert((unsigned)wordID);
  }
}

bool Lexicon::CorrectionIsAllowed(int wordID) const
{
  return noncorrectable_word_ids.find(wordID) == noncorrectable_word_ids.end();
}

} // namespace korektor
} // namespace ufal
