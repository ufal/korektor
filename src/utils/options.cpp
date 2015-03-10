// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <cstring>
#include <cstdlib>

#include "options.h"

namespace ufal {
namespace korektor {

const Options::Value Options::Value::none(Options::Value::NONE);
const Options::Value Options::Value::any(Options::Value::ANY);

bool Options::Parse(const unordered_map<string, Value>& allowed, int& argc, char**& argv, Map& options) {
  int args = 1;
  bool options_allowed = true;
  for (int argi = 1; argi < argc; argi++)
    if (argv[argi][0] == '-' && options_allowed) {
      if (argv[argi][1] == '-' && argv[argi][2] == '\0') {
        options_allowed = false;
        continue;
      }
      const char* option = argv[argi] + 1 + (argv[argi][1] == '-');
      const char* equal_sign = strchr(option, '=');

      string key = equal_sign ? string(option, equal_sign - option) : string(option);
      auto values = allowed.find(key);
      if (values == allowed.end()) return cerr << "Unknown option '" << argv[argi] << "'." << endl, false;

      string value;
      if (values->second.value == Value::NONE && equal_sign) return cerr << "Option '" << key << "' cannot have a value." << endl, false;
      if (values->second.value != Value::NONE) {
        if (equal_sign) {
          value.assign(equal_sign + 1);
        } else {
          if (argi + 1 == argc) return cerr << "Missing value for option '" << key << "'." << endl, false;
          value.assign(argv[++argi]);
        }
        if (!(values->second.value == Value::ANY || (values->second.value == Value::SET && values->second.allowed.count(value))))
          return cerr << "Option '" << key << "' cannot have value '" << value << "'." << endl, false;
      }
      options[key] = value;
    } else {
      argv[args++] = argv[argi];
    }

  argc = args;
  return true;
}

} // namespace korektor
} // namespace ufal
