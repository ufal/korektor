// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include <fstream>

#include "common.h"
#include "utf.h"

namespace ufal {
namespace korektor {

class UTF8InputStream
{
  ifstream ifs;
  bool closed;


  UTF8InputStream(const UTF8InputStream&) {}
  UTF8InputStream() {}

 public:

  UTF8InputStream(const string filename): ifs(filename.c_str()), closed(false)
  {
    if (ifs.is_open() == false)
    {
      cerr << "Can't open file " << filename << endl;
      exit(-123);
    }
  }

  bool ReadLineUS(u16string &us)
  {
    if (closed == true) return false;

    if (ifs.eof())
    {
      ifs.close();
      closed = true;
      return false;
    }
    else
    {
      string s;
      getline(ifs, s);

      if (s.length() > 0 && s[s.length() - 1] == '\n') s.erase(--s.end());
      if (s.length() > 0 && s[s.length() - 1] == '\r') s.erase(--s.end());
      if (s.length() > 1 && s[s.length() - 2] == '\r') s.erase(s.end() - 2, s.end() - 1);

      //utf8 text files created on Windows contain this character at a beginning of the file (and you don't want that...)

      us = UTF::UTF8To16(s);

      while ((!us.empty()) &&  *us.begin() >= 0xD800)
        us.erase(us.begin());

      return true;
    }
  }

  bool ReadLineString(string &s)
  {
    u16string us;
    bool ret = ReadLineUS(us);

    if (ret == true)
      s = UTF::UTF16To8(us);
    return ret;
  }
};

} // namespace korektor
} // namespace ufal
