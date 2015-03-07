// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "string_array.h"

namespace ufal {
namespace korektor {

StringArray::StringArray(istream &ifs)
{
  ifs.read((char*)&data_size, sizeof(uint32_t));

  data = new char[data_size];
  ifs.read(data, data_size);
  offsets = CompIncreasingArray(ifs);
}

StringArray::StringArray(vector<string> &vals)
{
  uint32_t bytes_needed = 0;
  vector<uint32_t> offs;

  for (size_t i = 0; i < vals.size(); i++)
  {
    offs.push_back(bytes_needed);
    bytes_needed += vals[i].length();
  }

  data = new char[bytes_needed];

  for (size_t i = 0; i < vals.size(); i++)
  {
    memcpy(&(data[offs[i]]), vals[i].c_str(), vals[i].length());
  }

  offsets = CompIncreasingArray(offs, bytes_needed - 1);

  data_size = bytes_needed;
}

void StringArray::WriteToStream(ostream &ofs)
{
  ofs.write((char*)&data_size, sizeof(uint32_t));

  ofs.write(data, data_size);

  offsets.WriteToStream(ofs);
}

} // namespace korektor
} // namespace ufal
