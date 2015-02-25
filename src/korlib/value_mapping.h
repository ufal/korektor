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

namespace ngramchecker {


class ValueMapping {
 private:
  vector<double> sorted_centers; ///< Sorted larger set mapped from a vector of values
  uint num_bits_per_value; ///< Bits per value

 public:

  /// @brief Get the bits per value
  ///
  /// @return Bits (integer)
  inline uint BitsPerValue() const
  {
    return num_bits_per_value;
  }

  /// @brief Get the value from the mapped set using the given index
  ///
  /// @param centerID Index in the sorted set
  /// @return Value from sorted set at a given index (double)
  inline double GetDouble(uint32_t centerID) const
  {
    return sorted_centers[centerID];
  }

  uint32_t GetCenterID(double value) const;

  void writeToStream(ostream &ofs) const;

  ValueMapping(vector<double> centers);

  ValueMapping(istream &ifs);

  ValueMapping() {}

  ValueMapping(vector<double> values, const uint32_t bits_per_value);

};

SP_DEF(ValueMapping);
}
