// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "error_model.h"

namespace ufal {
namespace korektor {

const float ErrorModel::impossible_cost = 1000;
const float ErrorModel::max_cost = 100;
const float ErrorModel::name_cost = 70;
const float ErrorModel::unknown_cost = 100;

} // namespace korektor
} // namespace ufal
