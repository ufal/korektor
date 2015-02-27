// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <unordered_map>

#include "common.h"
#include "error_model/error_model.h"
#include "utils/utf.h"
#include "utils/utils.h"

namespace ufal {
namespace korektor {

class hierarchy_node;
typedef shared_ptr<hierarchy_node> hierarchy_nodeP;

class hierarchy_node {
 private:
#if 0
  static u16string generateRandomString(unsigned min_length, unsigned range)
  {
    u16string ret;
    unsigned length = Utils::randomR(min_length, range);

    for (unsigned i = 0; i < length; i++)
      ret += central_chars[Utils::randomR(0, central_chars.length())];

    return ret;
  }
#endif

 public:
  static unordered_map<u16string, hierarchy_nodeP> hierarchy_map;
  static hierarchy_nodeP root;

  u16string signature;
  hierarchy_nodeP parent;
  vector<hierarchy_nodeP> children;
  unsigned num_governed_leaves;
  unsigned error_count;

  float error_prob;
  unsigned edit_distance;

  unsigned context_count;
  bool is_output_node;

#if 0
  void generate_spelling_error(u16string &misspelled, u16string &correct)
  {
    if (children.size() == 0)
    {
      //-------------------------------------GENERATE ERROR------------------------------------------
      if (signature.substr(0, 2) == UTF::UTF8To16("s_"))
      {
        //---------------------------------SUBSTITUTION--------------------------------------------
        u16string prefix = generateRandomString(0, 4);
        u16string suffix = generateRandomString(0, 4);

        misspelled = prefix;
        correct = prefix;

        misspelled += signature[2];
        correct += signature[3];
        misspelled += suffix;
        correct += suffix;
        return;
      }
      else if (signature.substr(0, 5) == UTF::UTF8To16("swap_"))
      {
        //------------------------------------------SWAP--------------------------------------------
        u16string prefix = generateRandomString(0, 4);
        u16string suffix = generateRandomString(0, 4);

        misspelled = prefix;
        correct = prefix;

        misspelled += signature[6];
        misspelled += signature[5];

        correct += signature[5];
        correct += signature[6];

        misspelled += suffix;
        correct += suffix;
        return;

      }
      else if (signature.substr(0, 2) == UTF::UTF8To16("i_"))
      {
        //----------------------------------------INSERT--------------------------------------------
        u16string prefix;
        u16string suffix;

        if (signature[3] != UChar(' '))
        {
          prefix = generateRandomString(0, 3);
          prefix += signature[3];
        }

        if (signature[4] != UChar(' '))
        {
          suffix += signature[4];
          suffix += generateRandomString(0, 3);
        }

        correct = prefix;
        correct += suffix;
        misspelled = prefix;
        misspelled += signature[2];
        misspelled += suffix;
        return;
      }
      else if (signature.substr(0, 2) == UTF::UTF8To16("d_"))
      {
        //-------------------------------------DELETE-----------------------------------------------
        u16string prefix;
        u16string suffix = generateRandomString(0, 4);

        if (signature[3] != UChar(' '))
        {
          prefix = generateRandomString(0, 3);
          prefix += signature[3];
        }

        correct = prefix;
        correct += signature[2];
        correct += suffix;

        misspelled = prefix;
        misspelled += suffix;
        return;
      }
    }
    else if (children.size() == 1)
    {
      children[0]->generate_spelling_error(misspelled, correct);
    }
    else
    {
      vector<unsigned> offsets;

      unsigned counter = 0;
      for (auto it = children.begin(); it != children.end(); it++)
      {
        offsets.push_back(counter);
        counter += (*it)->num_governed_leaves;
      }

      int r_num = Utils::randomR(0, counter);

      unsigned i = 0;
      while (i + 1 < offsets.size() && offsets[i + 1] < r_num) i++;

      children[i]->generate_spelling_error(misspelled, correct);
    }
  }
#endif

 private:
  hierarchy_node(const u16string &_signature): signature(_signature), num_governed_leaves(1), error_count(0), edit_distance(1), is_output_node(false)
  {}

 public:
  static bool ContainsNode(const u16string &_signature)
  {
    return hierarchy_map.find(_signature) != hierarchy_map.end();
  }

  static hierarchy_nodeP GetNode(const u16string &_signature)
  {
    return hierarchy_map.find(_signature)->second;
  }

  static hierarchy_nodeP create_SP(const u16string &_signature, hierarchy_nodeP &_parent, const u16string &context_chars)
  {
    //cerr << "constructing hierarchy node: " << UTF::UTF16To8(_signature) << endl;
    hierarchy_nodeP ret = hierarchy_nodeP(new hierarchy_node(_signature));
    ret->parent = _parent;
    if (_parent)
    {
      _parent->children.push_back(ret);

      if (_parent->children.size() > 1)
      {
        _parent->num_governed_leaves++;
        hierarchy_nodeP aux = _parent->parent;

        while (aux)
        {
          aux->num_governed_leaves++;
          aux = aux->parent;
        }
      }
    }

    hierarchy_map[_signature] = ret;

    for (unsigned i = 0; i < _signature.length(); i++)
    {
      if (_signature[i] == char16_t('.'))
      {
        u16string sig_copy = _signature;
        for (unsigned j = 0; j < context_chars.length(); j++)
        {
          sig_copy[i] = context_chars[j];

          if (ContainsNode(sig_copy) == false)
            hierarchy_node::create_SP(sig_copy, ret, context_chars);
        }
      }

    }

    //cerr << "construction finished: " << UTF::UTF16To8(_signature) << endl;
    return ret;
  }

  static void print_hierarchy_rec(hierarchy_nodeP &node, unsigned level, ostream &os)
  {
    for (unsigned i = 0; i < level; i++)
      os << "\t";

    string s = UTF::UTF16To8(node->signature);
    os << s << endl;

    for (auto it = node->children.begin(); it != node->children.end(); it++)
      print_hierarchy_rec(*it, level + 1, os);
  }

  static pair<hierarchy_nodeP, unsigned> read_hierarchy(string s, hierarchy_nodeP node, unsigned level)
  {
    //cerr << s << endl;
    unsigned new_level = 0;
    while (s[new_level] == '\t') new_level++;

    while (level >= new_level)
    {
      node = node->parent;
      level--;
    }

    FATAL_CONDITION(new_level > 0 && new_level < 10, "");

    FATAL_CONDITION(level + 1 == new_level, "");

    u16string signature = UTF::UTF8To16(s.substr(new_level));

    //cerr << "signature: " << UTF::UTF16To8(signature) << endl;

    hierarchy_nodeP new_node = hierarchy_nodeP(new hierarchy_node(signature));

    hierarchy_node::hierarchy_map.insert(make_pair(signature, new_node));

    node->children.push_back(new_node);
    new_node->parent = node;

    return make_pair(new_node, new_level);

  }

  static void ReadHierarchy(istream &is)
  {
    string s;
    if (Utils::SafeReadline(is, s))
    {
      FATAL_CONDITION(s == "root", "");

      hierarchy_node::root = hierarchy_nodeP(new hierarchy_node(UTF::UTF8To16("root")));
      hierarchy_map.insert(make_pair(root->signature, root));
      pair<hierarchy_nodeP, unsigned> node_level_pair = make_pair(root, 0);

      while (Utils::SafeReadline(is, s))
      {
        node_level_pair = read_hierarchy(s, node_level_pair.first, node_level_pair.second);
      }
    }
  }

  static void output_result_rec(hierarchy_nodeP &node, unsigned level, unsigned prop_level, float inherited_prob, u16string prop_signature, vector<pair<u16string, ErrorModelOutput>> &out_vec)
  {
//    if (level == 1)
//    {
//      if (node->is_output_node)
//      {
//        out_vec.push_back(make_pair(node->signature, ErrorModelOutput(node->edit_distance, node->error_prob)));
//        //ofs << UTF::UTF16To8(node->signature) << "\t" << node->edit_distance << "\t" << node->error_prob << endl;
//      }
//    }

    if (node->children.empty())
    {
      if (prop_level == 1 && node->children.empty() && node->is_output_node == false)
        return;

      if (node->is_output_node)
      {
        out_vec.push_back(make_pair(node->signature, ErrorModelOutput(node->edit_distance, node->error_prob)));
        //ofs << UTF::UTF16To8(node->signature) << "\t" << node->edit_distance << "\t" << node->error_prob << endl;
      }
      else
      {
        out_vec.push_back(make_pair(node->signature, ErrorModelOutput(node->edit_distance, inherited_prob)));
        //ofs << UTF::UTF16To8(node->signature) << "\t" << node->edit_distance << "\t" << inherited_prob /*<< "(" << UTF::UTF16To8(prop_signature) << ")"*/ << endl;
      }

    }

    float propagated_value;

    if (node->is_output_node)
    {
      cerr << "propagating: " << UTF::UTF16To8(node->signature) << endl;
      propagated_value = node->error_prob;
      prop_signature = node->signature;
      prop_level = level;
    }
    else
      propagated_value = inherited_prob;

    for (auto it = node->children.begin(); it != node->children.end(); it++)
    {
      output_result_rec((*it), level + 1, prop_level, propagated_value, prop_signature, out_vec);
    }
  }
};

} // namespace korektor
} // namespace ufal
