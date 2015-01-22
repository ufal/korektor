/*
 * DebugLexicon.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: indictree
 */

#include "common.hpp"
#include "helpers.hpp"
#include "Lexicon.hpp"

using namespace std;

int main() {
	std::vector<string> words{"mindless", "madness", "in", "manhattan"};
	Lexicon vocab = Lexicon::fromUTF8Strings(words);
	vocab.ArcsConsistencyCheck();
	vocab.PrintWords(cout, 2);
	return 0;
}


