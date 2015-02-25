// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file my_increasing_array.h
/// @class MyIncreasingArray my_increasing_array.h "my_increasing_array.h"
/// @brief Data structure for efficiently storing integer
///        arrays whose values are in an increasing order.

#pragma once

#include "common.h"

#include "my_packed_array.h"

namespace ngramchecker {

class MyIncreasingArray {
 private:
  uint32_t multiplier; ///< Multiplier for approximating function
  int32_t value_shift; ///< How much deviation from the original values
  MyPackedArray shifted_offsets; ///< Stored values

 public:

  /// @brief Get the size of the packed array
  ///
  /// @return Size of the packed array
  inline uint32_t GetSize() const
  {
    return shifted_offsets.GetSize();
  }

  /// @brief Get the array value at index
  ///
  /// @param i Array index
  /// @return Array value at given index
  inline uint32_t GetValueAt(uint32_t i) const
  {
    return ((i * multiplier) >> 10) + shifted_offsets.GetValueAt(i) + value_shift;
  }

  MyIncreasingArray(istream &ifs);

  void WriteToStream(ostream &ofs) const;

  MyIncreasingArray(vector<uint32_t> &vec);

};

SP_DEF(MyIncreasingArray);
}
