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
#include <unordered_set>

#include "common.h"
#include "error_hierarchy.h"
#include "utils/utf8_input_stream.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

u16string central_chars;
u16string context_chars;

unordered_set<char16_t> letters_set;

unordered_map<char16_t, char16_t> left_neighbour;
unordered_map<char16_t, char16_t> right_neighbour;
unordered_map<char16_t, char16_t> top_neighbour;
unordered_map<char16_t, char16_t> bottom_neighbour;

int CreateErrorHierarchy(const string &layout_file, const string &error_hierarchy_out)
{
  hierarchy_nodeP null_parent;
  hierarchy_nodeP root = hierarchy_node::create_SP(UTF::UTF8To16("root"), null_parent, context_chars);

  UTF8InputStream utf8_stream(layout_file);

  if (!utf8_stream.ReadLineUS(central_chars)) return -4;
  if (!utf8_stream.ReadLineUS(context_chars)) return -4;

  u16string diacritic_chars;
  if (!utf8_stream.ReadLineUS(diacritic_chars)) return -4;

  u16string no_diacritic_chars;
  if (!utf8_stream.ReadLineUS(no_diacritic_chars)) return -4;

  vector<u16string> keyboard_layout;

  u16string us;

  if (!utf8_stream.ReadLineUS(us)) return -4;
  keyboard_layout.push_back(us);

  if (!utf8_stream.ReadLineUS(us)) return -4;
  keyboard_layout.push_back(us);

  if (!utf8_stream.ReadLineUS(us)) return -4;
  keyboard_layout.push_back(us);

  if (!utf8_stream.ReadLineUS(us)) return -4;
  keyboard_layout.push_back(us);

  while (utf8_stream.ReadLineUS(us)) ;

  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < keyboard_layout[i].length(); j++)
    {
      //cerr << i << ", " << j << endl;
      char16_t curr_char = keyboard_layout[i][j];
      if (i > 0 && j < keyboard_layout[i - 1].length() && (! (i == 1 && j == 0)) )
      {
        top_neighbour[curr_char] = keyboard_layout[i - 1][j];
      }

      if (j > 0 && (!(i == 0 && j == 1)) )
      {
        left_neighbour[curr_char] = keyboard_layout[i][j - 1];
      }

      if (i < 3 && j < keyboard_layout[i + 1].length())
      {
        bottom_neighbour[curr_char] = keyboard_layout[i + 1][j];
      }

      if (j < keyboard_layout[i].length() - 1)
      {
        right_neighbour[curr_char] = keyboard_layout[i][j + 1];
      }
    }

  //---------------------------------------------SUBSTITUTIONS-----------------------------------------------

  hierarchy_nodeP substitutions = hierarchy_node::create_SP(UTF::UTF8To16("substitutions"), root, context_chars);

  hierarchy_nodeP hadj_sub = hierarchy_node::create_SP(UTF::UTF8To16("sub_hadj"), substitutions, context_chars);

  hierarchy_nodeP vadj_sub = hierarchy_node::create_SP(UTF::UTF8To16("sub_vadj"), substitutions, context_chars);

  for (unsigned i = 0; i < central_chars.length(); i++)
  {
    char16_t ch = central_chars[i];

    auto fit = left_neighbour.find(ch);
    if (fit != left_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("s_");
      signature += ch;
      signature += fit->second;

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, hadj_sub, context_chars);
      }
    }

    auto fit2 = right_neighbour.find(ch);
    if (fit2 != right_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("s_");
      signature += ch;
      signature += fit2->second;

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, hadj_sub, context_chars);
      }
    }

    auto fit3 = top_neighbour.find(ch);
    if (fit3 != top_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("s_");
      signature += ch;
      signature += fit3->second;

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, vadj_sub, context_chars);
      }
    }

    auto fit4 = bottom_neighbour.find(ch);
    if (fit4 != bottom_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("s_");
      signature += ch;
      signature += fit4->second;
      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, vadj_sub, context_chars);
      }
    }

  }

  hierarchy_nodeP missing_diac = hierarchy_node::create_SP(UTF::UTF8To16("sub_missing_diac"), substitutions, context_chars);
  hierarchy_nodeP redundant_diac = hierarchy_node::create_SP(UTF::UTF8To16("sub_redundant_diac"), substitutions, context_chars);

  for (unsigned i = 0; i < diacritic_chars.length(); i++)
  {
    u16string sig1 = UTF::UTF8To16("s_");
    sig1 += no_diacritic_chars[i];
    sig1 += diacritic_chars[i];

    if (!hierarchy_node::ContainsNode(sig1))
      hierarchy_nodeP missing = hierarchy_node::create_SP(sig1, missing_diac, context_chars);

    u16string sig2 = UTF::UTF8To16("s_");
    sig2 += diacritic_chars[i];
    sig2 += no_diacritic_chars[i];

    if (!hierarchy_node::ContainsNode(sig2))
      hierarchy_nodeP missing = hierarchy_node::create_SP(sig2, redundant_diac, context_chars);
  }

  for (unsigned i = 0; i < central_chars.length(); i++)
    for (unsigned j = 0; j < central_chars.length(); j++)
    {
      u16string sig = UTF::UTF8To16("s_");
      sig += central_chars[i];
      sig += central_chars[j];

      if (!hierarchy_node::ContainsNode(sig))
        hierarchy_nodeP ble = hierarchy_node::create_SP(sig, substitutions, context_chars);
    }

  //--------------------------------------INSERTIONS-------------------------------------------------------------

  hierarchy_nodeP insertions = hierarchy_node::create_SP(UTF::UTF8To16("insertions"), root, context_chars);

  hierarchy_nodeP hadj_insert = hierarchy_node::create_SP(UTF::UTF8To16("insert_hadj"), insertions, context_chars);
  hierarchy_nodeP vadj_insert = hierarchy_node::create_SP(UTF::UTF8To16("insert_vadj"), insertions, context_chars);

  for (unsigned i = 0; i < central_chars.length(); i++)
  {
    char16_t ch = central_chars[i];

    auto fit = left_neighbour.find(ch);
    if (fit != left_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("i_");
      signature += ch;
      signature += fit->second;
      signature += char16_t('.');

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, hadj_insert, context_chars);
      }

      u16string signature2 = UTF::UTF8To16("i_");
      signature2 += ch;
      signature2 += char16_t('.');
      signature2 += fit->second;

      if (!hierarchy_node::ContainsNode(signature2))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature2, hadj_insert, context_chars);
      }
    }

    auto fit2 = right_neighbour.find(ch);
    if (fit2 != right_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("i_");
      signature += ch;
      signature += fit2->second;
      signature += char16_t('.');

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, hadj_insert, context_chars);
      }

      u16string signature2 = UTF::UTF8To16("i_");
      signature2 += ch;
      signature2 += char16_t('.');
      signature2 += fit2->second;

      if (!hierarchy_node::ContainsNode(signature2))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature2, hadj_insert, context_chars);
      }
    }

    auto fit3 = top_neighbour.find(ch);
    if (fit3 != top_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("i_");
      signature += ch;
      signature += fit3->second;
      signature += char16_t('.');

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, vadj_insert, context_chars);
      }

      u16string signature2 = UTF::UTF8To16("i_");
      signature2 += ch;
      signature2 += char16_t('.');
      signature2 += fit3->second;

      if (!hierarchy_node::ContainsNode(signature2))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature2, vadj_insert, context_chars);
      }

    }

    auto fit4 = bottom_neighbour.find(ch);
    if (fit4 != bottom_neighbour.end())
    {
      u16string signature = UTF::UTF8To16("i_");
      signature += ch;
      signature += fit4->second;
      signature += char16_t('.');

      if (!hierarchy_node::ContainsNode(signature))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature, vadj_insert, context_chars);
      }

      u16string signature2 = UTF::UTF8To16("i_");
      signature2 += ch;
      signature2 += char16_t('.');
      signature2 += fit4->second;

      if (!hierarchy_node::ContainsNode(signature2))
      {
        hierarchy_nodeP node = hierarchy_node::create_SP(signature2, vadj_insert, context_chars);
      }

    }

  }

  for (unsigned i = 0; i < central_chars.length(); i++)
  {
    u16string signature = UTF::UTF8To16("i_");
    signature += central_chars[i];
    signature += UTF::UTF8To16("..");
    hierarchy_nodeP node = hierarchy_node::create_SP(signature, insertions, context_chars);
  }

  //------------------------------------DELETIONS------------------------------------------------------

  hierarchy_nodeP deletions = hierarchy_node::create_SP(UTF::UTF8To16("deletions"), root, context_chars);

  for (unsigned i = 0; i < central_chars.length(); i++)
  {
    u16string signature = UTF::UTF8To16("d_");
    signature += central_chars[i];
    signature += char16_t('.');
    hierarchy_node::create_SP(signature, deletions, context_chars);
  }

  //-----------------------------------SWAPS--------------------------------------------------------

  hierarchy_nodeP swaps = hierarchy_node::create_SP(UTF::UTF8To16("swaps"), root, context_chars);

  for (unsigned i = 0; i < central_chars.length(); i++)
    for (unsigned j = 0; j < central_chars.length(); j++)
    {
      u16string signature = UTF::UTF8To16("swap_");
      signature += central_chars[i];
      signature += central_chars[j];
      hierarchy_node::create_SP(signature, swaps, context_chars);
    }

  ofstream ofs;
  ofs.open(error_hierarchy_out.c_str());
  if (!ofs.is_open())
    return -2;

  hierarchy_node::print_hierarchy_rec(root, 0, ofs);
  ofs.close();
  return 0;
}

} // namespace korektor
} // namespace ufal
