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

namespace ufal {
namespace korektor {

class Hash {
 public:

  static void Combine(size_t &seed, unsigned value)
  {
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }
};

} // namespace korektor
} // namespace ufal
