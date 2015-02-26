// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <unordered_map>

#include "common.h"

namespace ufal {
namespace korektor {

typedef unordered_map<uint32_t, pair<u16string, double> > Similar_Words_Map;

} // namespace korektor
} // namespace ufal
