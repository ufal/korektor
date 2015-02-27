// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file utils.h
/// @class Utils utils.h "utils.h"
/// @brief Utility functions

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class Utils {
 public:

  static void HashCombine(size_t &seed, unsigned value)
  {
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  /// @brief random number in a range
  static double RandomNumber(double min, double max);

  /// @brief random number in a range
  static int randomR(int lowest, int range);

  /// @brief minimal number of bits needed for representing the given unsigned int value
  /// @param value Integer input
  /// @return Required bits in integer
  static uint32_t BitsNeeded(uint32_t value)
  {
    uint32_t ret = 0;
    uint32_t pom = 1;
    while (pom <= value)
    {
      ret++;
      pom = pom << 1;
    }

    return ret;
  }
};

} // namespace korektor
} // namespace ufal
