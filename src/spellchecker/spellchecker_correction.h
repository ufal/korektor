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

struct SpellcheckerCorrection {
  enum type_t { NONE, GRAMMAR, SPELLING };

  type_t type;
  u16string correction;
  vector<u16string> alternatives;

  SpellcheckerCorrection(type_t type = NONE) : type(type) {}
};

} // namespace korektor
} // namespace ufal
