// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef STRING_PIECE_H
#define STRING_PIECE_H

#include <cstring>

#include "StdAfx.h"

namespace ngramchecker {

struct StringPiece {
  const char* str;
  size_t len;

  StringPiece() : str(nullptr), len(0) {}
  StringPiece(const char* str) : str(str), len(strlen(str)) {}
  StringPiece(const char* str, size_t len) : str(str), len(len) {}
  StringPiece(const std::string& str) : str(str.c_str()), len(str.size()) {}
};

} // namespace ngramchecker

#endif
