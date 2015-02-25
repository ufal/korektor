/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file ErrorModel.hpp
/// @brief Data structure and relevant functions for the implementation of error model

#ifndef ERRORMODEL_HPP_
#define ERRORMODEL_HPP_

#include "StdAfx.h"

namespace ngramchecker {

/// @class ErrorModelOutput ErrorModel.hpp "ErrorModel.hpp"
/// @brief Class for error model output. It contains two members, namely edit distance and the cost involved.
class ErrorModelOutput {
 public:
  uint32_t edit_dist; ///< Edit distance
  float cost; ///< Cost involved in substitution/insertion/deletion/swap

  /// @brief Constructor initialization
  ///
  /// @param _edit_dist Edit distance
  /// @param _cost Cost
  ErrorModelOutput(uint32_t _edit_dist, float _cost): edit_dist(_edit_dist), cost(_cost) {}

  /// @brief Default constructor
  ErrorModelOutput(): edit_dist(0), cost(0.0f) {}
};

//
/// @class ErrorModel ErrorModel.hpp "ErrorModel.hpp"
/// @brief Abstract base class for Error model.
///
/// @todo do we actually want the functions here to be virtual? As long as there is just a single implementation, it's useless.
class ErrorModel
{
 public:
  static const float impossible_cost/* = 1000*/;
  static const float max_cost;
  static const float name_cost;
  static const float unknown_cost;

  inline float UnknownWordCost() { return unknown_cost; }

  inline float NameCost() { return name_cost; }

  virtual ErrorModelOutput SubstitutionCost(char16_t char1, char16_t letter2, bool ignore_case = false) = 0;

  virtual ErrorModelOutput InsertionCost(char16_t inserted_char, char16_t previous_char, char16_t next_char) = 0;

  virtual ErrorModelOutput SwapCost(char16_t first_char, char16_t second_char) = 0;

  virtual ErrorModelOutput DeletionCost(char16_t current_char, char16_t previous_char) = 0;

  virtual uint32_t MinOperationEditDistance() = 0;

  virtual bool StringsAreIdentical(const u16string &s1, const u16string &s2) = 0;

  virtual string ToString() = 0;

};

SP_DEF(ErrorModel);

//ErrorModelTypoP errorModel;

}


#endif /* ERRORMODEL_HPP_ */
