// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"

#include "my_increasing_array.h"

namespace ngramchecker {

typedef pair<uint32_t, uint32_t> CompIA_First_Last_IndexPair;

class CompIncreasingArray {
 private:
  uint32_t log2_size_of_parts; ///< Exponent of 2
  uint32_t bit_mask; ///< Maximum value
  uint32_t num_values; ///< Array size
  uint32_t last_val_last_index; ///< -
  vector<MyIncreasingArrayP> mia_vec; ///< Each vector element points to the subgroup of the original data


 public:

  /// @brief Get the number of items in the data sequence
  ///
  /// @return Number of items (unsigned integer)
  inline uint32_t GetSize() const
  {
    return num_values;
  }

  /// @brief Get the value at a given index
  ///
  /// @param i Index
  /// @return The value at the given index (unsigned integer)
  /// @todo optimize this instead of / and % use >> and &
  inline uint32_t GetValueAt(uint32_t i) const
  {
    return mia_vec[i >> log2_size_of_parts]->GetValueAt(i & bit_mask);
  }

  /// @brief Get the first and last index in the group where the given index is located
  ///
  /// @param i Index
  /// @return Pair <first (integer), last (integer)>
  inline CompIA_First_Last_IndexPair GetFirstLastIndexPair(uint32_t i) const
  {

    uint32_t first = GetValueAt(i);

    uint32_t last;
    if (i < GetSize() - 1)
    {
      last = GetValueAt(i + 1);
      if (last == 0)
      {
        return make_pair(1, 0);
      }
      last--;
    }
    else
      last = last_val_last_index;

    assert(last <= last_val_last_index);

    return make_pair(first, last);
  }

  void WriteToStream(ostream &ofs) const;

  /// @todo Undefined routine
  void WriteToStreamNew(ostream &ofs) const;

  CompIncreasingArray() {}

  CompIncreasingArray(istream &ifs);

  CompIncreasingArray(vector<uint32_t> &val, uint32_t _last_val_last_index);
};

SP_DEF(CompIncreasingArray);
}
