// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file factor_list.h
/// @brief Data structure for factor

#pragma once

#include "common.h"

/// @struct FactorList factor_list.h "factor_list.h"
/// @brief Data structure for factors.
struct FactorList {
  enum { MAX_FACTORS = 4 };

  unsigned factors[MAX_FACTORS];
  float emission_costs[MAX_FACTORS];
};
