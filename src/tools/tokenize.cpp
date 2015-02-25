/// @file tokenize.cpp
/// @brief Simple tokenizer
///
///

#include <vector>
#include <map>
#include <string>
#include <stdint.h>
#include "StdAfx.h"

#include "korlib/Token.hpp"
#include "korlib/Tokenizer.hpp"

#ifndef uint
#define uint uint32_t
#endif

using namespace std;
using namespace ngramchecker;

// floating point number:  [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
// date: yyyy-mm-dd        (19|20)\d\d([- /.])(0[1-9]|1[012])\2(0[1-9]|[12][0-9]|3[01])
// date: dd-mm-yyyy        (0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])[- /.](19|20)\d\d
// date: mm-dd-yyyy        (0[1-9]|[12][0-9]|3[01])[- /.](0[1-9]|1[012])[- /.](19|20)\d\d
// email:                  \b[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}\b
// url:                    \b(([\w-]+://?|www[.])[^\s()<>]+(?:\([\w\d]+\)|([^[:punct:]\s]|/)))
// number:                 \b[0-9]+\b
// sentence end:           ([.?!:]) [[:upper:]]
// token:                  \b([.<.>/?;:'"\\\\\\[{\\]}=+-_()*&^%$#@!]|\w+)\b


int main(int argc, char** argv)
{
  Tokenizer tokenizer;
  string s;

  while (std::getline(cin, s))
  {
    tokenizer.TokenizeToStream(MyUtils::utf8_to_utf16(s), cout);
  }

  return 0;
}
