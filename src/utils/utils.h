// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file utils.h
/// @class Utils utils.h "utils.h"
/// @brief Utility functions

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class Utils {
 public:

  static void HashCombine(size_t &seed, unsigned value)
  {
    seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  /// @brief Utility for writing a string to binary stream
  /// @param ofs Output stream
  /// @param s Input string
  static void WriteString(ostream &ofs, const string &s);

  /// @brief Utility for reading a string value from a binary stream
  /// @param ifs Input stream
  /// @return Read string
  /// @todo This routine should be better renamed
  static string ReadString(istream &ifs);

  static uint32_t Read_uint32_t(istream &is);

  /// @brief random number in a range
  static double RandomNumber(double min, double max);

  /// @brief random number in a range
  static int randomR(int lowest, int range);

  /// @brief minimal number of bits needed for representing the given unsigned int value
  /// @param value Integer input
  /// @return Required bits in integer
  static uint32_t BitsNeeded(uint32_t value)
  {
    uint32_t ret = 0;
    uint32_t pom = 1;
    while (pom <= value)
    {
      ret++;
      pom = pom << 1;
    }

    return ret;
  }

  /// @brief wrapper around std::getline - eventually removes '\r' from the end - i.e. when a text file created on Windows is being opened on Linux
  static bool SafeReadline(istream &istr, string &str);

  /// @brief split the string in s into toks, characters contained in delims form a set of delimiters (i.e. they are not included into toks)
  static void Split(vector<string> &toks, const string &s, const string &delims)
  {
    toks.clear();

    string::const_iterator segment_begin = s.begin();
    string::const_iterator current = s.begin();
    string::const_iterator string_end = s.end();

    while (true)
    {
      if (current == string_end || delims.find(*current) != string::npos || *current == '\r')
      {
        if (segment_begin != current)
          toks.push_back(string(segment_begin, current));

        if (current == string_end || *current == '\r')
          break;

        segment_begin = current + 1;
      }

      current++;
    }

  }

  /// @brief String to float conversion
  /// @param Input string
  /// @return float
  static float my_atof(const string &str)
  {
    float ret = (float)std::atof(str.c_str());

    //if (str.empty() || (ret == 0.0f && str[0] != '0'))
    //  throw std::bad_cast("error in my_atof: value cannot be converted to float!");

    return ret;
  }

  /// @brief String to integer conversion
  /// @param Input string
  /// @return int
  static int my_atoi(const string &str)
  {
    int ret = std::atoi(str.c_str());

    //if (str.empty() || ret == 0 && str[0] != '0')
    //  throw std::bad_cast("error in my_atoi: value cannot be converted to int!");

    return ret;
  }
};

} // namespace korektor
} // namespace ufal
