// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file packed_array.h
/// @class PackedArray packed_array.h "packed_array.h"
/// @brief Data structure for the implementation of memory efficient array

#pragma once

#include "common.h"

namespace ufal {
namespace korektor {

class PackedArray {
 private:
  uint32_t bits_per_value; ///< bits per value
  uint32_t num_values; ///< number of items in the array
  uint32_t num_bytes; ///< size of the array
  unsigned char* data; ///< pointer to the actual data
  uint32_t output_mask; ///< the maximum value that the packed array can hold



 public:

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
    uint64_t bit = uint64_t(index) * bits_per_value;
    uint32_t byte_pointer = bit >> 3;
    uint32_t bit_offset = bit % 8;

    uint64_t retVal = *((uint64_t*)&(data[byte_pointer]));
    retVal = (retVal >> bit_offset) & output_mask;

    return (uint32_t)retVal;
  }


  ~PackedArray();

  PackedArray() { data = NULL; }

  /// @brief Initialize the array using another array
  PackedArray(const PackedArray& val);

  /// @brief Copy the array using assignment operator
  PackedArray& operator=(const PackedArray& val);

  /// @brief Initialize the array using binary stream
  PackedArray(istream &ifs);

  /// @brief Initialize the array from file
  PackedArray(const string filename);

  /// @brief Write the array to output stream
  void WriteToStream(ostream &ofs) const;

  /// @brief Write the array to file
  void SaveToFile(const string filename) const;

  /// @brief Initialize the array from a vector of values
  PackedArray(const vector<uint32_t> &values);
};

} // namespace korektor
} // namespace ufal
