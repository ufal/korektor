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
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace ufal {
namespace korektor {

// Printf-like logging function.
inline int eprintf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = vfprintf(stderr, fmt, ap);
  va_end(ap);
  return res;
}

// Printf-like exit function.
inline void runtime_errorf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

#define SP_DEF(cl_name) typedef shared_ptr<cl_name> cl_name##P

using namespace std;

} // namespace korektor
} // namespace ufal
