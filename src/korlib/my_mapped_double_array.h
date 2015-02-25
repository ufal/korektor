// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#ifndef MYMAPPEDDOUBLEARRAY_HPP_
#define MYMAPPEDDOUBLEARRAY_HPP_

#include "common.h"

#include "value_mapping.h"
#include "my_packed_array.h"

namespace ngramchecker {

class MyMappedDoubleArray {
 private:
  ValueMapping value_mapping;
  MyPackedArray mpa;

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

  MyMappedDoubleArray(vector<double> &values, uint32_t bits_per_value);

  MyMappedDoubleArray(istream &ifs);
};

SP_DEF(MyMappedDoubleArray);
}

#endif /* MYMAPPEDDOUBLEARRAY_HPP_ */
