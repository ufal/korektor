// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file common.h
/// @brief Common header file

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#define FATAL_CONDITION(expr, error_text) if (!(expr)) { fprintf(stderr, "FATAL ERROR: The condition ( %s ) failed!\n", #expr); fflush(stderr); exit(1); }

#define SP_DEF(cl_name) typedef shared_ptr<cl_name> cl_name##P

namespace ufal {
namespace korektor {

using namespace std;

} // namespace korektor
} // namespace ufal
