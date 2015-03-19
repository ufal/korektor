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

struct Constants {
  static const double prunning_constant;

  static const uint32_t sentence_start_id;
  static const uint32_t sentence_end_id;
  static const uint32_t unknown_word_id;
};

} // namespace korektor
} // namespace ufal
