// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "my_increasing_array.h"
#include "comp_increasing_array.h"
#include "utils.h"

namespace ufal {
namespace korektor {

/// @brief Write the data structure to output stream
///
/// @param ofs Output stream
void CompIncreasingArray::WriteToStream(ostream &ofs) const
{
  MyUtils::WriteString(ofs, "CIA");
  uint32_t num_parts = mia_vec.size();
  ofs.write((char*)&num_parts, sizeof(uint32_t));
  ofs.write((char*)&log2_size_of_parts, sizeof(uint32_t));
  ofs.write((char*)&num_values, sizeof(uint32_t));
  ofs.write((char*)&last_val_last_index, sizeof(uint32_t));

  for (uint32_t i = 0; i < mia_vec.size(); i++)
  {
    mia_vec[i]->WriteToStream(ofs);
  }
}

/// @brief Constructor initialization from input stream/file
///
/// @param Input stream
CompIncreasingArray::CompIncreasingArray(istream &ifs)
{
  string check_string = MyUtils::ReadString(ifs);

  FATAL_CONDITION(check_string == "CIA", check_string);

  uint32_t num_parts;

  ifs.read((char*)&num_parts, sizeof(uint32_t));
  ifs.read((char*)&log2_size_of_parts, sizeof(uint32_t));
  ifs.read((char*)&num_values, sizeof(uint32_t));
  ifs.read((char*)&last_val_last_index, sizeof(uint32_t));

  for (uint32_t i = 0; i < num_parts; i++)
  {
    mia_vec.push_back(MyIncreasingArrayP(new MyIncreasingArray(ifs)));
  }

  bit_mask = (1 << log2_size_of_parts) - 1;
}

/// @brief Divide the large sequence of unsigned integers into small groups and store them
///        individually in MyIncreasingArray data structure.
///
/// @param val Vector of increasing integer array
/// @param _last_val_last_index -
/// @todo The variable _last_val_last_index is not used
CompIncreasingArray::CompIncreasingArray(vector<uint32_t> &val, uint32_t _last_val_last_index)
{
  num_values = val.size();
  log2_size_of_parts = 12;
  bit_mask = (1 << log2_size_of_parts) - 1;

  uint32_t first = 0;
  uint32_t last = min((int)(1 << log2_size_of_parts) - 1, (int)(val.size()) - 1);

  while (first < num_values)
  {
    vector<uint32_t> part;
    for (uint32_t i = first; i <= last; i++)
      part.push_back(val[i]);
    mia_vec.push_back(MyIncreasingArrayP(new MyIncreasingArray(part)));
    first += 1 << log2_size_of_parts;
    last += 1 << log2_size_of_parts;
    if (last >= num_values)
      last = num_values - 1;
  }

  for (uint32_t i = 0; i < num_values; i++)
  {
    FATAL_CONDITION(val[i] == GetValueAt(i), "CIA");
  }

  last_val_last_index = _last_val_last_index;
}

} // namespace korektor
} // namespace ufal
