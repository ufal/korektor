/*
 * DebugSnippets.cpp
 *
 *  Created on: Jan 22, 2015
 *      Author: indictree
 */

#include "common.hpp"
#include "helpers.hpp"
#include "Lexicon.hpp"
#include "Morphology.hpp"


using namespace std;

typedef std::map<string, uint>::iterator mapit1;

void step_morphology(void);
void step_value_mapping(void);

int main()
{
	//step_morphology();
	step_value_mapping();
	return 0;
}


void step_morphology() {
	ifstream ifs;
	ifs.open(MORPHLEXFILE, ios::binary);
	if (ifs.is_open()) {
		Morphology m(ifs);

		// factor names
		std::map<string, uint> factormap = m.GetFactorMap();
		mapit1 it1;
		for (it1 = factormap.begin(); it1 != factormap.end(); it1++) {
			cout << "[" << it1->first << ", " << it1->second << "]" << endl;
		}

		ifs.close();
	}
}

void step_value_mapping()
{
	std::vector<double> values{7, 3,8};
	uint32_t bpv = 4;
	ValueMapping vm(values, 4);
}
