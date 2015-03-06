// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "version.h"

namespace ufal {
namespace korektor {

// Returns current version.
version version::current() {
  return {1, 9, 0, "devel"};
}

} // namespace korektor
} // namespace ufal
