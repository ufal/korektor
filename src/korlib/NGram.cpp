/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file NGram.cpp
/// @class NGram NGram.hpp "NGram.hpp"

#include "NGram.hpp"

namespace ngramchecker {

/// @brief Initialize N-gram from another N-gram
///
/// @param val Instance of N-gram
NGram::NGram(const NGram &val):
  order(val.order), prob(val.prob), backoff(val.backoff)
{
  word_ids = new uint32_t[order];
  memcpy(word_ids, val.word_ids, sizeof(uint32_t) * order);
}

/// @brief Initialize N-gram from another N-gram through assignment operator
///
/// @param val Instance of N-gram
NGram& NGram::operator=(const NGram& val)
{
  if (&val != this)
  {
    order = val.order;
    backoff = val.backoff;
    prob = val.prob;

    if (word_ids != NULL)
      delete[] word_ids;

    word_ids = new uint32_t[order];
    memcpy(word_ids, val.word_ids, sizeof(uint32_t) * order);

  }

  return *this;
}

/// @brief Initialize N-gram through constructor arguments
///
/// @param order N-gram order
/// @param IDs Pointer to word ids
/// @param _prob N-gram probability
/// @param _backoff Backoff weight
NGram::NGram(uint32_t order, uint32_t *IDs, double _prob, double _backoff):
  order(order), prob(_prob), backoff(_backoff), word_ids(IDs)
{}

bool NGram::ZeroBackoff() const
{
  //assert(1 == 0); //To be implemented!
  return false;
}

/// @brief Destructor
NGram::~NGram()
{
  if (word_ids != NULL)
    delete[] word_ids;
}

/// @brief N-gram to string representation
///
/// @return N-gram string representation (string)
string NGram::ToString()
{
  stringstream strs;
  strs << "order: " << order << endl;
  strs << "next_word: " << word_ids[0] << endl;
  strs << "history (youngest -> oldest):";
  for (uint32_t i = 1; i < order; i++)
    strs << " " << word_ids[i];
  strs << endl << "prob = " << prob << ", bow = " << backoff << endl;
  return strs.str();
}
}
