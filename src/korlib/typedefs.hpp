/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file typedefs.hpp
/// @brief Type definitions
/// @copyright Copyright (c) 2012, Charles University in Prague
/// All rights reserved.

#ifndef _TYPEDEFS_HPP_
#define _TYPEDEFS_HPP_

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 ulong64;
typedef signed __int64 long64;
#else
typedef unsigned long long ulong64;
typedef signed long long long64;

#endif

#include <memory>

using namespace std;

namespace ngramchecker {
typedef shared_ptr<string> stringP;
typedef shared_ptr<u16string> u16stringP;

typedef shared_ptr<vector<uint32_t> > vectorP_uint32_t;
typedef unordered_map<uint32_t, pair<u16stringP, double> > Similar_Words_Map;


enum StagePosibility_type { MultiFactor, Letter };
enum capitalization_type { all_upper_case, all_lower_case, first_upper_case, weird };

}
#endif
