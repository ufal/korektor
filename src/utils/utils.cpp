// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file utils.cpp
/// @brief Utility functions

#include <cmath>
#include <ctime>
#include <iostream>

#include "utils.h"

namespace ufal {
namespace korektor {

void Utils::WriteString(ostream &ofs, const string &s)
{
  FATAL_CONDITION(uint16_t(s.length()) == s.length(), "");
  uint16_t len = s.length();
  ofs.write((char*)&len, sizeof(uint16_t));
  ofs.write(s.data(), sizeof(char) * len);
}

string Utils::ReadString(istream &ifs)
{
  uint16_t len;
  ifs.read((char*)&len, sizeof(uint16_t));
  char* chars = new char[len + 1];
  ifs.read(chars, len);
  chars[len] = 0;
  string ret = chars;
  delete[] chars;

  return ret;
}

uint32_t Utils::Read_uint32_t(istream &is)
{
  uint32_t ret;
  is.read((char*)&ret, sizeof(uint32_t));
  return ret;
}

bool Utils::SafeReadline(istream &istr, string &str)
{
  if (!std::getline(istr, str))
    return false;
  else
  {
    if (str.length() > 0 && str[str.length() - 1] == '\r')
    {
      str.erase(str.length() - 1);
    }
    return true;
  }
}

/// @brief Generates a random number between the given range
///
/// @param min Lower bound
/// @param max Upper bound
/// @return Random number
double Utils::RandomNumber(double min, double max)
{
  return ((double)rand() / RAND_MAX) * (max - min) + min;
}

int Utils::randomR(int lowest, int range)
{
  uint32_t ret = lowest+uint32_t(range*(rand()/(RAND_MAX + (float)1.0)));
  return ret;
}

} // namespace korektor
} // namespace ufal
