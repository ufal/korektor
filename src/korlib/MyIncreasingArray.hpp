/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file MyIncreasingArray.hpp
/// @class MyIncreasingArray MyIncreasingArray.hpp "MyIncreasingArray.hpp"
/// @brief Data structure for efficiently storing integer
///        arrays whose values are in an increasing order.
///

#ifndef MYINCREASINGARRAY_HPP_
#define MYINCREASINGARRAY_HPP_

#include "StdAfx.h"

#include "MyPackedArray.hpp"

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

#endif /* MYINCREASINGARRAY_HPP_ */
