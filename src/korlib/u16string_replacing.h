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

class u16stringReplacing {
  u16string ustring;
  int offset;
  unsigned first_allowed_start;
 public:
  u16stringReplacing(const u16string &_ustring): ustring(_ustring), offset(0), first_allowed_start(0) {}

  void Replace(unsigned start, unsigned length, const u16string &replacement)
  {
    assert(start >= first_allowed_start);
    ustring.replace(start + offset, length, replacement);
    offset += replacement.length() - length;
    first_allowed_start = start + length;
  }

  u16string GetResult()
  {
    return ustring;
  }
};

} // namespace korektor
} // namespace ufal
