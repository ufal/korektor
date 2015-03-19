// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <sstream>

#include "unilib/version.h"
#include "version.h"

namespace ufal {
namespace korektor {

// Returns current version.
version version::current() {
  return {1, 9, 0, "devel"};
}

// Returns multi-line formated version and copyright string.
string version::version_and_copyright(const string& other_libraries) {
  ostringstream info;

  auto korektor = version::current();
  auto unilib = unilib::version::current();

  info << "Korektor version " << korektor.major << '.' << korektor.minor << '.' << korektor.patch
       << (korektor.prerelease.empty() ? "" : "-") << korektor.prerelease
       << " (using UniLib " << unilib.major << '.' << unilib.minor << '.' << unilib.patch
       << (other_libraries.empty() ? "" : " and ") << other_libraries << ")\n"
          "Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty of\n"
          "Mathematics and Physics, Charles University in Prague, Czech Republic.";

  return info.str();
}

} // namespace korektor
} // namespace ufal
