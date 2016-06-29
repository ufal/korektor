// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file packed_array.cpp
/// @brief Implementation of memory efficient array

#include <cstring>

#include "packed_array.h"
#include "utils/io.h"

namespace ufal {
namespace korektor {

/// @brief Get the number of items in the packed array
///
/// @return Number of items
uint32_t PackedArray::GetSize() const
{
  return num_values;
}

/// @brief Get the number of bytes required to store the packed array
///
/// @return Number of bytes
uint32_t PackedArray::GetNumberOfBytes() const
{
  return num_bytes;
}

/// @brief Get bits per value
///
/// @return Bits per value (integer)
uint32_t PackedArray::GetBitsPerValue() const
{
  return bits_per_value;
}

/// @brief Get output mask
///
/// @return Output mask (integer)
uint32_t PackedArray::GetOutputMask() const {
  return output_mask;
}

PackedArray::~PackedArray()
{
  if (data != NULL)
    delete[] data;
}

/// @brief Initialize the array using another array
///
/// @param val Packed array
PackedArray::PackedArray(const PackedArray& val)
{
  bits_per_value = val.bits_per_value;
  num_bytes = val.num_bytes;
  num_values = val.num_values;
  output_mask = val.output_mask;

  data = new unsigned char[num_bytes];

  memcpy(data, val.data, val.num_bytes * sizeof(unsigned char));

  //cerr << "PackedArray: copy constructor\n";
}

/// @brief Copy the array using assignment operator
///
/// @param val Packed array
/// @return Reference to the copied array
PackedArray& PackedArray::operator=(const PackedArray& val)
{
  if (this != &val)
  {
    unsigned char* new_data = new unsigned char[val.num_bytes];

    memcpy(new_data, val.data, val.num_bytes * sizeof(unsigned char));

    bits_per_value = val.bits_per_value;
    num_bytes = val.num_bytes;
    num_values = val.num_values;
    output_mask = val.output_mask;

    if (data != NULL)
      delete[] data;
    data = new_data;

  }

  //cerr << "PackedArray: assignemt operator\n";
  return *this;
}

/// @brief Initialize the array using binary stream
///
/// @param ifs Input stream
PackedArray::PackedArray(istream &ifs)
{
  if (IO::ReadString(ifs) != "MPA")
    runtime_failure("Cannot load PackedArray, file is corrupted!");

  ifs.read((char*)&bits_per_value, sizeof(uint32_t));
  ifs.read((char*)&num_bytes, sizeof(uint32_t));
  ifs.read((char*)&num_values, sizeof(uint32_t));
  ifs.read((char*)&output_mask, sizeof(uint32_t));

  data = new unsigned char[num_bytes];

  ifs.read((char*)data, sizeof(unsigned char) * num_bytes);

}

/// @brief Write the array to output stream
///
/// @param ofs Output stream
void PackedArray::WriteToStream(ostream &ofs) const
{
  IO::WriteString(ofs, "MPA");
  ofs.write((char*)&bits_per_value, sizeof(uint32_t));
  ofs.write((char*)&num_bytes, sizeof(uint32_t));
  ofs.write((char*)&num_values, sizeof(uint32_t));

  ofs.write((char*)&output_mask, sizeof(uint32_t));
  ofs.write((char*)data, sizeof(unsigned char) * num_bytes);

}

/// @brief Initialize the array from a vector of values
///
/// @param values Vector of values
PackedArray::PackedArray(const vector<uint32_t> &values)
{
  uint32_t max = 0;
  for (uint32_t i = 0; i < values.size(); i++)
  {
    if (values[i] > max) max = values[i];
  }

  uint32_t num_bits = 0;
  uint32_t pom = 1;
  while (max + 1 > pom)
  {
    pom = pom << 1;
    num_bits++;
  }

  num_bytes = (num_bits * values.size() + 7) / 8;

  bits_per_value = num_bits;
  num_values = values.size();

  output_mask = 0;
  for (uint32_t i = 0; i < num_bits; i++)
  {
    output_mask = output_mask << 1;
    output_mask++;
  }

  data = new unsigned char[num_bytes];

  for (uint32_t i = 0; i < num_bytes; i++)
    data[i] = 0;

  uint32_t byte_pointer;
  uint32_t bit_pointer;
  uint64_t curr_value;
  unsigned char* ukaz;
  for (uint32_t i = 0; i < values.size(); i++)
  {
    int bites_left = bits_per_value;

    // moves to the next byte once the values
    // fill up the current byte
    byte_pointer = (uint64_t(i) * num_bits) >> 3;

    bit_pointer = (i * num_bits) % 8;

    curr_value = values[i];

    curr_value = curr_value << bit_pointer;
    ukaz = (unsigned char*)&curr_value;
    bites_left -= 8 - bit_pointer;
    data[byte_pointer] = data[byte_pointer] | ukaz[0];

    uint32_t j = 1;
    while (bites_left > 0)
    {
      data[byte_pointer + j] = data[byte_pointer + j] | ukaz[j];
      bites_left -= 8;
      j++;
    }
  }

  for (uint32_t i = 0; i < values.size(); i++)
  {
    assert(values[i] == GetValueAt(i));
  }
}

} // namespace korektor
} // namespace ufal
