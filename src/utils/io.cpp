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

#include <iostream>

#include "io.h"

namespace ufal {
namespace korektor {

void IO::WriteString(ostream &ofs, const string &s)
{
  assert(uint16_t(s.length()) == s.length());
  uint16_t len = s.length();
  ofs.write((char*)&len, sizeof(uint16_t));
  ofs.write(s.data(), sizeof(char) * len);
}

string IO::ReadString(istream &ifs)
{
  uint16_t len;
  ifs.read((char*)&len, sizeof(uint16_t));

  string ret(len, ' ');
  ifs.read((char*) ret.data(), len);
  return ret;
}

int32_t IO::ReadInt32(istream &is)
{
  int32_t ret;
  is.read((char*)&ret, sizeof(int32_t));
  return ret;
}

uint32_t IO::ReadUInt32(istream &is)
{
  uint32_t ret;
  is.read((char*)&ret, sizeof(uint32_t));
  return ret;
}


bool IO::ReadLine(istream &istr, string &str)
{
  if (!std::getline(istr, str))
    return false;

  if (!str.empty() && str.back() == '\r')
    str.pop_back();

  return true;
}

void IO::Split(const string& text, char delim, vector<string>& tokens) {
  tokens.clear();
  if (text.empty()) return;

  string::size_type index = 0;
  for (string::size_type next; (next = text.find(delim, index)) != string::npos; index = next + 1)
    tokens.emplace_back(text, index, next - index);

  tokens.emplace_back(text, index);
}

void IO::Split(const string& text, const char* delims, vector<string>& tokens) {
  tokens.clear();
  if (text.empty()) return;

  string::size_type index = 0;
  for (string::size_type next; (next = text.find_first_of(delims, index)) != string::npos; index = next + 1)
    tokens.emplace_back(text, index, next - index);

  tokens.emplace_back(text, index);
}


} // namespace korektor
} // namespace ufal
