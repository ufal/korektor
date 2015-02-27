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
#include "packed_array.h"
#include "value_mapping.h"

namespace ufal {
namespace korektor {

class MappedDoubleArray {
 private:
  ValueMapping value_mapping;
  PackedArray mpa;

 public:
  inline uint32_t GetSize() const
  {
    return mpa.GetSize();
  }

  inline double GetValueAt(uint32_t index) const
  {
    return value_mapping.GetDouble(mpa.GetValueAt(index));
  }

  void WriteToStream(ostream &ofs) const;

  MappedDoubleArray(vector<double> &values, uint32_t bits_per_value);

  MappedDoubleArray(istream &ifs);
};

SP_DEF(MappedDoubleArray);

} // namespace korektor
} // namespace ufal
