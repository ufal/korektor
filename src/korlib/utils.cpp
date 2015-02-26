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

void MyUtils::WriteString(ostream &ofs, const string &s)
{
  FATAL_CONDITION(uint16_t(s.length()) == s.length(), "");
  uint16_t len = s.length();
  ofs.write((char*)&len, sizeof(uint16_t));
  ofs.write(s.data(), sizeof(char) * len);
}

string MyUtils::ReadString(istream &ifs)
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

uint32_t MyUtils::Read_uint32_t(istream &is)
{
  uint32_t ret;
  is.read((char*)&ret, sizeof(uint32_t));
  return ret;
}

bool MyUtils::SafeReadline(istream &istr, string &str)
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
double MyUtils::RandomNumber(double min, double max)
{
  return ((double)rand() / RAND_MAX) * (max - min) + min;
}

int MyUtils::randomR(int lowest, int range)
{
  uint32_t ret = lowest+uint32_t(range*(rand()/(RAND_MAX + (float)1.0)));
  return ret;
}

double MyUtils::minus_log_10_rand01()
{
  double rand_num = -log10(rand() / (float)RAND_MAX);
  return rand_num;
}


void MyUtils::cerr_vector_string(const vector<string> &vec)
{
  for (uint32_t i = 0; i < vec.size(); i++)
  {
    if (i > 0) cerr << endl;
    cerr << vec[i];
  }
  cerr << endl;
}

/// @brief Returns time string
/// @return Time string
string MyUtils::GetTimeString()
{
  string ret;

  time_t rawtime;

  time ( &rawtime );

  string time_str = ctime(&rawtime);

  ret = time_str.substr(4, 3) + "-" + time_str.substr(8, 2) + "_" + time_str.substr(11, 2) + "-" + time_str.substr(14, 2);

  return ret;
}

/// @brief String to Boolean conversion
/// @param str The values are "true" or "false"
/// @return Returns 0 or 1 based on the given input string
bool MyUtils::StrToBool(string str)
{
  if (str == "true")
    return true;
  else if (str == "false")
    return false;
  else
  {
    cerr << "str -> bool conversion failed!" << endl;
    exit(1);
  }
}

/// @brief String to double conversion
/// @param str input string
/// @return Value in double
double MyUtils::StrToDouble(string str)
{
  return my_atof(str);
}

/// @brief String to integer conversion
/// @param str input string
/// @return Intger value
uint32_t MyUtils::StrToInt(string str)
{
  return my_atoi(str);
}

/// @brief Boolean to String conversion
/// @param value boolean
/// @return Returns "true" or "false" based on the input value
string MyUtils::BoolToString(bool value)
{
  if (value)
    return "true";
  else
    return "false";
}
/// @brief Check whether the given string contains letter
/// @param ustr String
/// @return boolean
bool MyUtils::ContainsLetter(const u16string &ustr)
{
  for (auto&& uchar : ustr)
    if (MyUTF::is_alpha(uchar))
      return true;

  return false;
}

} // namespace korektor
} // namespace ufal
