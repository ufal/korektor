/*
 * DebugCompIncreasingArray.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: indictree
 */

#include "common.hpp"
#include "helpers.hpp"
#include "CompIncreasingArray.hpp"

#define MAX_NUMBERS 10000

int main() {
	std::vector<uint32_t> vec;
	for (int i = 0; i < MAX_NUMBERS; i++) {
		vec.push_back(i);
	}
	uint32_t last_v = 9999;
	CompIncreasingArray cia(vec, last_v);
	return 0;
}

