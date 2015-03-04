// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <iostream>

#include "bit_array.h"
#include "utils/io.h"

namespace ufal {
namespace korektor {

BitArray::BitArray(istream &ifs)
{
  if (IO::ReadString(ifs) != "MBA")
    runtime_errorf("Cannot load BitArray, file is corrupted!");


  output_mask.push_back(0);
  uint32_t curr_mask = 0;
  for (unsigned i = 1; i <= 32; i++)
  {
    curr_mask = curr_mask << 1;
    curr_mask++;
    output_mask.push_back(curr_mask);
  }


  ifs.read((char*)&num_bytes, sizeof(uint32_t));

  //TODO: initialization of the output masks;
  //ifs.read((char*)&output_mask, sizeof(uint32_t));

  data = new unsigned char[num_bytes];

  ifs.read((char*)data, sizeof(unsigned char) * num_bytes);

}

void BitArray::WriteToStream(ostream &ofs) const
{
  IO::WriteString(ofs, "MBA");
  ofs.write((char*)&num_bytes, sizeof(uint32_t));

  //TODO: output masks!
  //ofs.write((char*)&output_mask, sizeof(uint32_t));
  ofs.write((char*)data, sizeof(unsigned char) * num_bytes);

}

BitArray::BitArray(const vector<pair<uint32_t, unsigned> > &values)
{
  uint32_t bits_needed = 0;
  for (uint32_t i = 0; i < values.size(); i++)
  {
    bits_needed += values[i].second;
  }

  num_bytes = (bits_needed + 7) / 8;

  output_mask.push_back(0);
  uint32_t curr_mask = 0;
  for (unsigned i = 1; i <= 32; i++)
  {
    curr_mask = curr_mask << 1;
    curr_mask++;
    output_mask.push_back(curr_mask);
  }

  data = new unsigned char[num_bytes];

  for (uint32_t i = 0; i < num_bytes; i++)
    data[i] = 0;

  uint32_t byte_pointer;
  uint32_t bit_pointer;
  uint32_t bit_position = 0;
  uint64_t curr_value;
  unsigned char* ukaz;
  for (uint32_t i = 0; i < values.size(); i++)
  {
    int bites_left = values[i].second;
    byte_pointer = bit_position >> 3;
    bit_pointer = bit_position % 8;

    curr_value = values[i].first;

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

    bit_position += values[i].second;
  }

  bit_position = 0;
  for (uint32_t i = 0; i < values.size(); i++)
  {
    assert(GetValueAt(bit_position, values[i].second) == values[i].first);
    bit_position += values[i].second;
  }

}

SP_DEF(BitArray);

} // namespace korektor
} // namespace ufal
