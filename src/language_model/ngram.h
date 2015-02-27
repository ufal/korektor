// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file ngram.h
/// @class NGram ngram.h "ngram.h"

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class NGram {

 public:
  uint32_t order;  ///< N-gram order
  double prob;    ///< Probabilities
  double backoff; ///< Backoff weights
  uint32_t *word_ids; ///< Word ids

  NGram() { word_ids = NULL; }
  NGram(const NGram &val);

  NGram& operator=(const NGram& val);

  NGram(uint32_t order, uint32_t *IDs, double _prob, double _backoff);

  /// @brief Constructor initialization
  ///
  /// @param order N-
  NGram(uint32_t _order): order(_order)
  {
    word_ids = new uint32_t[order];
  }

  /// @brief Get word ID
  ///
  /// @param index Index
  /// @return Word ID
  inline uint32_t GetWordID_New(uint32_t index) const //index = 0 ~ next word, index = 1 ~ most recent history etc...
  {
    return word_ids[index];
  }

  bool ZeroBackoff() const;

  ~NGram();
};

/// @struct NGram_compare ngram.h "ngram.h"
/// @brief Data structure for comparing N-grams
struct NGram_compare: std::less<NGram>
{
  bool operator()(const NGram &x, const NGram &y)
  {
    unsigned min_order = x.order < y.order ? x.order : y.order;

    for (unsigned i = 0; i < min_order; i++)
    {
      if (x.word_ids[i] < y.word_ids[i])
        return true;
      else if (x.word_ids[i] > y.word_ids[i])
        return false;
    }

    if (x.order < y.order)
      return true;
    else
      return false;
  }
};

} // namespace korektor
} // namespace ufal
