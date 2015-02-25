// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file my_packed_array.h
/// @class MyPackedArray my_packed_array.h "my_packed_array.h"
/// @brief Data structure for the implementation of memory efficient array

#ifndef _MY_PACKED_ARRAY_HPP_
#define _MY_PACKED_ARRAY_HPP_

#include "common.h"
#include "utils.h"

namespace ngramchecker {

class MyPackedArray {
 private:
  uint32_t bits_per_value; ///< bits per value
  uint32_t num_values; ///< number of items in the array
  uint32_t num_bytes; ///< size of the array
  unsigned char* data; ///< pointer to the actual data
  uint32_t output_mask; ///< the maximum value that the packed array can hold



 public:

  void PrintBits() const;

  uint32_t GetSize() const;

  uint32_t GetNumberOfBytes() const;

  uint32_t GetBitsPerValue() const;

  uint32_t GetOutputMask() const;

  /// @brief Get the array value at index
  ///
  /// @param index Valid index
  /// @return Array value at index
  inline uint32_t GetValueAt(uint32_t index) const
  {
    uint32_t bit = index * bits_per_value;
    uint32_t byte_pointer = bit >> 3;
    uint32_t bit_offset = bit % 8;

    uint64_t retVal = *((uint64_t*)&(data[byte_pointer]));
    retVal = (retVal >> bit_offset) & output_mask;

    return (uint32_t)retVal;
  }


  ~MyPackedArray();

  MyPackedArray() { data = NULL; }

  /// @brief Initialize the array using another array
  MyPackedArray(const MyPackedArray& val);

  /// @brief Copy the array using assignment operator
  MyPackedArray& operator=(const MyPackedArray& val);

  /// @brief Initialize the array using binary stream
  MyPackedArray(istream &ifs);

  /// @brief Initialize the array from file
  MyPackedArray(const string filename);

  /// @brief Write the array to output stream
  void WriteToStream(ostream &ofs) const;

  /// @brief Write the array to file
  void SaveToFile(const string filename) const;

  /// @brief Initialize the array from a vector of values
  MyPackedArray(const vector<uint32_t> &values);

  static void TestIt()
  {

    vector<uint32_t> vals;

    for (uint i = 0; i < 10000000; i++)
    {
      vals.push_back(MyUtils::randomR(0, 10000000));
    }

    MyPackedArray mpa = MyPackedArray(vals);

    for (uint j = 0; j < 300; j++)
    {
      for (uint i = 0; i < 10000000; i++)
      {
        FATAL_CONDITION(vals[i] == mpa.GetValueAt(i), "");
      }
    }
    cerr << "Test ok!" << endl;
  }
};

}

#endif //_MY_PACKED_ARRAY_HPP_
