// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file utils.cpp
/// @brief Utility functions

#include <cmath>
#include <ctime>
#include <iostream>

#include "utils.h"

namespace ufal {
namespace korektor {

/// @brief Generates a random number between the given range
///
/// @param min Lower bound
/// @param max Upper bound
/// @return Random number
double Utils::RandomNumber(double min, double max)
{
  return ((double)rand() / RAND_MAX) * (max - min) + min;
}

int Utils::randomR(int lowest, int range)
{
  uint32_t ret = lowest+uint32_t(range*(rand()/(RAND_MAX + (float)1.0)));
  return ret;
}

} // namespace korektor
} // namespace ufal
