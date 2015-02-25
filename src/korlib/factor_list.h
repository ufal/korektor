/// @file factor_list.h
/// @brief Data structure for factor

#pragma once

#include "common.h"

/// @struct FactorList factor_list.h "factor_list.h"
/// @brief Data structure for factors.
struct FactorList {
  uint factors[4];
  float emission_costs[4];
};
