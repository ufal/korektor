// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "capitalization_type.h"
#include "utils.h"

namespace ufal {
namespace korektor {

capitalization_type GetCapitalizationType(const u16string& ustr)
{
  if (MyUtils::IsUpperCase(ustr[0]))
  {
    for (unsigned i = 1; i < ustr.length(); i++)
    {
      if (MyUtils::IsUpperCase(ustr[i]) == false)
      {
        if (i > 1)
          return WEIRD;
        else
        {
          for (unsigned j = 2; j < ustr.length(); j++)
          {
            if (MyUtils::IsUpperCase(ustr[j]))
              return WEIRD;
          }
        }
        return FIRST_UPPER_CASE;
      }
    }
    return ALL_UPPER_CASE;
  }
  else
  {
    for (unsigned i = 1; i < ustr.length(); i++)
    {
      if (MyUtils::IsUpperCase(ustr[i]))
        return WEIRD;
    }
    return ALL_LOWER_CASE;
  }
}

} // namespace korektor
} // namespace ufal
