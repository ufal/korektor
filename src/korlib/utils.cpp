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

#include "utils.h"
#include <time.h>

namespace ngramchecker {

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

capitalization_type MyUtils::Get_u16string_capitalization_type(const u16string &ustr)
{
  capitalization_type ct = all_lower_case;

  if (MyUtils::IsUpperCase(ustr[0]))
  {
    ct = first_upper_case;

    for (uint i = 1; i < ustr.length(); i++)
    {
      if (MyUtils::IsUpperCase(ustr[i]) == false)
      {
        if (i > 1)
          return weird;
        else
        {
          for (uint j = 2; j < ustr.length(); j++)
          {
            if (MyUtils::IsUpperCase(ustr[j]))
              return weird;
          }
        }
        //otherwise the ct value is first upper case
        return first_upper_case;
      }
    }

    return all_upper_case;
  }
  else
  {
    for (uint i = 1; i < ustr.length(); i++)
    {
      if (MyUtils::IsUpperCase(ustr[i]))
        return weird;
    }

    return all_lower_case;
  }

}

}
