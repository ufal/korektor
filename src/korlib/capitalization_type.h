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

enum capitalization_type { ALL_UPPER_CASE, ALL_LOWER_CASE, FIRST_UPPER_CASE, WEIRD };

capitalization_type GetCapitalizationType(const u16string& ustr);

} // namespace korektor
} // namespace ufal
