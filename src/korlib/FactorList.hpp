/// @file FactorList.hpp
/// @brief Data structure for factor

#pragma once

#include "StdAfx.h"

/// @struct FactorList FactorList.hpp "FactorList.hpp"
/// @brief Data structure for factors.
struct FactorList {
  uint factors[4];
  float emission_costs[4];
};
