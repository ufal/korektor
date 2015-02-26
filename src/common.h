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

#ifdef WIN32
#pragma warning(disable : 4996)
#pragma warning(disable : 4503)
#pragma warning(disable : 4244)
#endif

//STL:
#include <map>
#include <vector>
#include <set>
#include <string>
#include <cstdlib>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <cassert>
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#define FATAL_CONDITION(expr, error_text) if (!(expr)) { std::cerr << "FATAL ERROR: The condition ( " << #expr << " ) failed. " << error_text << std::endl; exit(1); }

#define SP_DEF(cl_name) typedef shared_ptr<cl_name> cl_name##P

namespace ufal {
namespace korektor {

using namespace std;

} // namespace korektor
} // namespace ufal

#include "korlib/typedefs.h"
#include "korlib/constants.h"
