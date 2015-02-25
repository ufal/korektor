/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file common.h
/// @brief Common header file
/// @copyright Copyright (c) 2012, Charles University in Prague
/// All rights reserved.

#ifndef COMMONHEADER_HPP_
#define COMMONHEADER_HPP_

#ifdef WIN32
#pragma warning(disable : 4996)
#pragma warning(disable : 4503)
#pragma warning(disable : 4244)
#endif

//STL:
#include <map>
#include <vector>
#include <set>
#include <string>
//#include <ctime>
#include <cstdlib>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <cassert>
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <memory>

//boost:
//#include <boost/lexical_cast.hpp>
//#include <boost/algorithm/string.hpp>
//#include <boost/functional/hash.hpp>

using namespace std;

#include "korlib/typedefs.h"
#include "korlib/constants.h"

using namespace ngramchecker;

#ifndef uint
#define uint uint32_t
#endif

#define SP_DEF(cl_name) typedef shared_ptr<cl_name> cl_name##P

#define triple(A, B, C) pair<A, pair<B, C> >
#define make_triple(A, B, C) std::make_pair(A, std::make_pair(B, C))

#define FIRST(A) A.first
#define SECOND(A) A.second.first
#define THIRD(A) A.second.second



//#pragma warning( disable : 4800 )
//#pragma warning( disable : 4503 )

#endif /* COMMONHEADER_HPP_ */
