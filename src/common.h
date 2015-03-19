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
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ufal {
namespace korektor {

#define runtime_failure(message) exit((cerr << message << endl, 1))

#define SP_DEF(cl_name) typedef shared_ptr<cl_name> cl_name##P

using namespace std;

} // namespace korektor
} // namespace ufal
