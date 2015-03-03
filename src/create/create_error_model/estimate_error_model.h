// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <cmath>
#include <unordered_map>

#include "common.h"
#include "error_hierarchy.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

const unsigned MINIMAL_ERROR_COUNT = 4;
const float error_count_normalization = 100;

class EstimateErrorModel
{
  hierarchy_nodeP hierarchy_root;
  unordered_map<u16string, uint32_t> context_map;

  bool estimate_error_model_rec(const hierarchy_nodeP &node)
  {
    if (node->children.empty())
    {
      u16string context;

      if (node->signature.substr(0, 5) == UTF::UTF8To16("swap_"))
      {
        context += node->signature[6];
        context += node->signature[5];
      }
      else if (node->signature[0] == char16_t('s'))
      {
        context += node->signature[2];

      }
      else if (node->signature[0] == char16_t('i'))
      {
        context += node->signature[3];
        context += node->signature[2];
        context += node->signature[4];

      }
      else if (node->signature[0] == char16_t('d'))
      {
        context += node->signature[3];
      }
      else
      {
        cerr << "invalid signature: " << UTF::UTF16To8(node->signature) << endl;
        return false;
      }

      if (context_map.find(context) == context_map.end())
      {
        //if (node->error_count > 0)
        //  cerr << "context not found: " << UTF::UTF16To8(context) << endl;
        node->context_count = node->error_count;
        return false;
      }
      else
      {
        //if (node->error_count > 0)
        //  cerr << "---------------------context found" << endl;
      }

      node->context_count = context_map[context];

      if (node->error_count > MINIMAL_ERROR_COUNT)
      {
        node->error_prob = - log10((node->error_count * error_count_normalization) / context_map[context]);
        //out_nodes.push_back(node);
        node->is_output_node = true;
        cerr << "output leaf: " << UTF::UTF16To8(node->signature) << endl;
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      assert(node->error_count == 0);
      unsigned context_count = 0;
      unsigned error_count = 0;

      for (auto it = node->children.begin(); it != node->children.end(); it++)
      {
        if (estimate_error_model_rec(*it) == false)
        {
          context_count += (*it)->context_count;
          error_count += (*it)->error_count;
        }
      }

      node->context_count = context_count;
      node->error_count = error_count;

      if (error_count > MINIMAL_ERROR_COUNT)
      {
        node->error_prob = -log10((error_count * error_count_normalization) / context_count);

        cerr << "node: " << UTF::UTF16To8(node->signature) << ", context_count = " << node->context_count << ", error_count = " << node->error_count << endl;
        node->is_output_node = true;
        return true;
      }
      else
      {
        return false;
      }
    }
  }

 public:
  EstimateErrorModel(hierarchy_nodeP _hierarchy_root, const unordered_map<u16string, uint32_t> &_context_map):
    hierarchy_root(_hierarchy_root), context_map(_context_map) {}

  void Estimate()
  {
    estimate_error_model_rec(hierarchy_root);
  }
};

} // namespace korektor
} // namespace ufal
