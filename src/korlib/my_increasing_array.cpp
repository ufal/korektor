// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file my_increasing_array.cpp
/// @brief Data structure for efficiently storing integer
///        arrays whose values are in an increasing order.

#include "my_increasing_array.h"
#include "my_packed_array.h"

namespace ngramchecker {

/// @brief Initialize the constructor from input stream
///
/// @param ifs Input stream
MyIncreasingArray::MyIncreasingArray(istream &ifs):
  multiplier(MyUtils::Read_uint32_t(ifs)),
  value_shift((int32_t)MyUtils::Read_uint32_t(ifs)),
  shifted_offsets(ifs)
{}

/// @brief Write the array to the output stream
///
/// @param ofs Output stream
void MyIncreasingArray::WriteToStream(ostream &ofs) const
{

  ofs.write((char*)&multiplier, sizeof(uint32_t));
  //ofs.write((char*)&denominator, sizeof(uint32_t));
  ofs.write((char*)&value_shift, sizeof(int32_t));
  shifted_offsets.WriteToStream(ofs);
}

/// @brief Initializing the construction of increasing array from vector
///
/// @param vec Vector of integer values
MyIncreasingArray::MyIncreasingArray(vector<uint32_t> &vec)
{
  uint32_t max_val = vec.back();
  uint32_t min_val = vec[0];

  for (uint32_t i = 0; i < vec.size() - 1; i++)
  {
    FATAL_CONDITION(vec[i] <= vec[i + 1], "");
  }

  double ratio = (double)(max_val - min_val) / (double)(vec.size() - 1);

  uint32_t denominator = 1 << 10;
  multiplier = (uint32_t)(denominator * ratio);

  vector<int> voffsets;

  int min_offset = 10000000;

  for (uint32_t i = 0; i < vec.size(); i++)
  {
    int offs = vec[i] - ((i * multiplier) >> 10) - min_val;
    voffsets.push_back(offs);

    if (offs < min_offset)
      min_offset = offs;

  }

  vector<uint32_t> sh_offsets;

  for (uint32_t i = 0; i < voffsets.size(); i++)
  {
    assert(voffsets[i] - min_offset >= 0);

    uint32_t sh_offset = voffsets[i] - min_offset;
    sh_offsets.push_back(sh_offset);
  }

  shifted_offsets = MyPackedArray(sh_offsets);
  value_shift = min_val + min_offset;

  //cerr << "MyIncreasingArray created: bitsPerValue = " << shifted_offsets->GetBitsPerValue() << endl;
  //cerr << "MyPackedArray would need " << BitsNeeded(max_val - min_val) << " bits\n----\n";

  for (uint32_t i = 0; i < vec.size(); i++)
  {
    WARNING(vec[i] == GetValueAt(i), "MIA i = " << i << ", vec[i] = " << vec[i] << ", GetVal[i] = " << GetValueAt(i));
  }
}

}
