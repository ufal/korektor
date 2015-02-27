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
#include "utf.h"

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

  static double minus_log_10_rand01();

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

  static bool CaseInsensitiveIsEqual(const u16string &str1, const u16string &str2)
  {
    if (str1.length() != str2.length())
      return false;

    for (unsigned i = 0; i < str1.length(); i++)
    {
      if (UTF::tolower(str1[i]) != UTF::tolower(str1[i]))
        return false;
    }

    return true;
  }

  //outputting a vector of strings into cerr
  static void cerr_vector_string(const vector<string> &vec);

  static string GetTimeString();

  /// @brief String to Boolean conversion
  ///
  /// @param str The values are "true" or "false"
  /// @return Returns 0 or 1 based on the given input string
  static bool StrToBool(string str);

  /// @brief String to double conversion
  ///
  /// @param str input string
  /// @return Value in double
  static double StrToDouble(string str);

  /// @brief String to integer conversion
  ///
  /// @param str input string
  /// @return Intger value
  static uint32_t StrToInt(string str);

  /// @brief Boolean to String conversion
  ///
  /// @param value boolean
  /// @return Returns "true" or "false" based on the input value
  static string BoolToString(bool value);


  /// @brief UTF-16 to UTF-8 conversion
  /// @return UTF-8 string
  static string utf16_to_utf8(const u16string &ustr)
  {
    return UTF::utf16_to_utf8(ustr);
  }

  /// @brief UTF-8 to UTF-16 conversion
  /// @return UTF-16 string
  static u16string utf8_to_utf16(const string &str)
  {
    return UTF::utf8_to_utf16(str);
  }

  /// @brief Check whether the character is in uppercase
  /// @return boolean
  static bool IsUpperCase(char16_t uchar)
  {
    return UTF::tolower(uchar) != uchar;
  }

  /// @brief Check whether the character is in lowercase
  /// @return boolean
  static bool IsLowerCase(char16_t uchar)
  {
    return UTF::toupper(uchar) != uchar;
  }

  /// @brief Change the character to lowercase
  /// @return Lowercase character
  static char16_t ToLower(char16_t uchar)
  {
    return UTF::tolower(uchar);
  }

  /// @brief Change the character to uppercase
  /// @return Uppercase character
  static char16_t ToUpper(char16_t uchar)
  {
    return UTF::toupper(uchar);
  }

  /// @brief Check whether the given string contains letter
  /// @param ustr String
  /// @return boolean
  static bool ContainsLetter(const u16string &ustr);

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

  /// @brief Integer to string conversion
  /// @param Input integer
  /// @return string
  static string my_itoa(int val)
  {
    static char buffer[200];
    sprintf(buffer, "%d", val);
    return string(buffer);
  }

};

///hashing function for u16string
struct u16string_hash
{
  size_t operator()(const u16string &val) const
  {
    size_t seed = 0;
    for (size_t i = 0; i < val.length(); i++)
    {
      Utils::HashCombine(seed, val[i]);
    }

    return seed;
  }
};

} // namespace korektor
} // namespace ufal