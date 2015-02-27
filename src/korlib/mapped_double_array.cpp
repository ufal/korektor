// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "mapped_double_array.h"

namespace ufal {
namespace korektor {

void MappedDoubleArray::WriteToStream(ostream &ofs) const
{
  value_mapping.writeToStream(ofs);
  mpa.WriteToStream(ofs);
}

MappedDoubleArray::MappedDoubleArray(vector<double> &values, uint32_t bits_per_value)
{
  value_mapping = ValueMapping(values, bits_per_value);

  vector<uint32_t> int_vals;

  for (uint32_t i = 0; i < values.size(); i++)
  {
    int_vals.push_back(value_mapping.GetCenterID(values[i]));
  }

  mpa = PackedArray(int_vals);
}

MappedDoubleArray::MappedDoubleArray(istream &ifs):
  value_mapping(ifs), mpa(ifs)
{}

} // namespace korektor
} // namespace ufal
