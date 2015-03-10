// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <initializer_list>
#include <unordered_map>
#include <unordered_set>

#include "common.h"

namespace ufal {
namespace korektor {

class Options {
 public:
  typedef unordered_map<string, string> Map;

  struct Value {
    enum value_t { NONE, ANY, SET };
    value_t value;
    unordered_set<string> allowed;

    Value(initializer_list<string> allowed) : value(SET), allowed(allowed) {}
    static const Value none;
    static const Value any;

   private:
    Value(value_t value) : value(value) {}
  };

  // Parse options according to option_values. If successful, argv is reordered so
  // that non-option arguments are placed in argv[1] to argv[argc-1]. The '--'
  // indicates end of option arguments (as usual).
  static bool Parse(const unordered_map<string, Value>& allowed, int& argc, char**& argv, Map& options);
};

} // namespace korektor
} // namespace ufal
