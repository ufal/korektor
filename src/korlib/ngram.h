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

#ifndef NGRAM_HPP_
#define NGRAM_HPP_

#include "common.h"
#include "utils.h"

namespace ngramchecker {

//class NGram;

//typedef shared_ptr<vector<NGramP> > vectorP_NGramP;

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

  string ToString();
};



/// @struct NGram_iequal_to ngram.h "ngram.h"
/// @brief Binary function for checking whether 2 N-grams are equal
struct NGram_iequal_to
: std::binary_function<NGram, NGram, bool>
{
  bool operator()(const NGram &x, const NGram &y)
  {
    if (x.order != y.order)
      return false;


    for (uint32_t i = 0; i < x.order; i++)
    {
      if (x.GetWordID_New(i) != y.GetWordID_New(i))
        return false;
    }

    return true;
  }
};


struct NGram_ihash: std::unary_function<NGram, std::size_t>
{
  std::size_t operator()(const NGram &x)
  {
    std::size_t seed = 0;

    MyUtils::HashCombine(seed, x.order);

    for (uint32_t i = 0; i < x.order; i++)
    {
      MyUtils::HashCombine(seed, x.GetWordID_New(i));
    }

    return seed;
  }
};

/// @struct NGram_compare ngram.h "ngram.h"
/// @brief Data structure for comparing N-grams
struct NGram_compare: std::less<NGram>
{
  bool operator()(const NGram &x, const NGram &y)
  {
    uint min_order = min(x.order, y.order);

    for (uint i = 0; i < min_order; i++)
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

}

#endif /* NGRAM_HPP_ */
