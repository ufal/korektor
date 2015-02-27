// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class IO {
 public:
  /// @brief Utility for writing a string to binary stream
  /// @param ofs Output stream
  /// @param s Input string
  static void WriteString(ostream &ofs, const string &s);

  /// @brief Utility for reading a string value from a binary stream
  /// @param ifs Input stream
  /// @return Read string
  /// @todo This routine should be better renamed
  static string ReadString(istream &ifs);

  static int32_t ReadInt32(istream &is);

  static uint32_t ReadUInt32(istream &is);

  /// @brief wrapper around std::getline - eventually removes '\r' from the end - i.e. when a text file created on Windows is being opened on Linux
  static bool ReadLine(istream &istr, string &str);

  /// @brief split the string in s into toks, with specified delimiter
  static void Split(const string& text, char delim, vector<string>& tokens);

  /// @brief split the string in s into toks, characters contained in delims form a set of delimiters (i.e. they are not included into toks)
  static void Split(const string& text, const char* delims, vector<string>& tokens);
};

} // namespace korektor
} // namespace ufal
