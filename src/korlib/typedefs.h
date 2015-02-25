// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file typedefs.h
/// @brief Type definitions

#pragma once

#include <memory>

using namespace std;

namespace ngramchecker {
typedef shared_ptr<string> stringP;
typedef shared_ptr<u16string> u16stringP;

typedef shared_ptr<vector<uint32_t> > vectorP_uint32_t;
typedef unordered_map<uint32_t, pair<u16stringP, double> > Similar_Words_Map;


enum StagePosibility_type { MultiFactor, Letter };
enum capitalization_type { all_upper_case, all_lower_case, first_upper_case, weird };

}
