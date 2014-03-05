// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef RESPONSE_GENERATOR_H
#define RESPONSE_GENERATOR_H

#include <unordered_map>

#include "StdAfx.h"

#include "StringPiece.h"

namespace ngramchecker {

class ResponseGenerator {
 public:
  virtual ~ResponseGenerator() {}

  virtual bool generate() = 0;
  virtual StringPiece current() = 0;
  virtual void consume(size_t length) = 0;
};

} // namespace ngramchecker

#endif
