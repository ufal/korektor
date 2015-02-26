// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file my_static_string_array.h
/// @class MyStaticStringArray my_static_string_array.h "my_static_string_array.h"
/// @brief Class for storing array of strings as data

#pragma once

#include <cstring>

#include "common.h"
#include "comp_increasing_array.h"

namespace ufal {
namespace korektor {

class MyStaticStringArray {
  char* data; ///< Pointer to string data
  uint32_t data_size; ///< Size of the data
  CompIncreasingArray offsets;

 public:

  /// @brief Get the data size
  ///
  /// @return Data size
  uint32_t GetSize()
  {
    return offsets.GetSize();
  }

  /// @brief Get the string value at a given index
  ///
  /// @param index Index value
  /// @return String value at the index
  string GetStringAt(size_t index)
  {
    CompIA_First_Last_IndexPair index_pair = offsets.GetFirstLastIndexPair(index);

    return string(data + index_pair.first, index_pair.second - index_pair.first + 1);
  }

  /// @brief Initialize the array from input stream
  ///
  /// @param ifs Input stream
  MyStaticStringArray(istream &ifs);

  /// @brief Initialize the array from a vector of strings
  ///
  /// @param vals Vector of strings
  MyStaticStringArray(vector<string> &vals);

  /// @brief Write the array to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs);

  /// @brief Destructor
  ~MyStaticStringArray()
  {
    delete[] data;
  }

  /// @brief Initialize the array from another static string array
  ///
  /// @param val MyStaticStringArray
  MyStaticStringArray(const MyStaticStringArray &val)
  {
    offsets = val.offsets;
    data_size = val.data_size;

    data = new char[data_size];
    memcpy(data, val.data, data_size);
  }

  /// @brief Copy the static array through assignment operator (copy constructor)
  ///
  /// @param val MyStaticStringArray
  /// @return MyStaticStringArray
  MyStaticStringArray& operator=(const MyStaticStringArray &val)
  {
    if (this != &val)
    {
      char* new_data = new char[val.data_size];
      memcpy(new_data, val.data, val.data_size);
      data_size = val.data_size;
      offsets = val.offsets;
      delete[] data;
      data = new_data;
    }

    return *this;
  }
};

SP_DEF(MyStaticStringArray);

} // namespace korektor
} // namespace ufal
