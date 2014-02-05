#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <map>

typedef uint32_t uint;

#include "utils.hpp"

using namespace std;

string processTag(const string &tag)
{
	string ret;

	char first = tag[0];

	if (first == 'C' || first == 'A' || first == 'N' || first == 'P')
		ret = tag.substr(0, 5);
	else
	if (first == 'V')
		ret = tag.substr(0, 4) + tag.substr(7, 2) + tag.substr(11, 1);
	else
	if (first == 'R')
		ret = tag.substr(0, 2) + tag.substr(4, 1);
	else
	if (first == 'D' || first == 'I' || first == 'J' || first == 'T')
		ret = tag.substr(0, 2);
	else
	if (first == 'X')
		ret = "<unk>";
	else
	{
		fprintf(stderr, "No rule for tag %s\n", tag.c_str());
		exit(1);
	}
  
	for (uint i = 0; i < ret.length(); i++)
	{
		if (ret[i] == 'X')
			ret[i] = '-';
	}
  
	return ret;
}

struct Ending 
{
	string tag;
	uint paradigm_id;
	bool flagNeg;
	bool flagNej;
	string ending;
};

vector<vector<Ending>> endings;

void writeEntry(const string &form, const string &lemma, const string &tag, ofstream &ofs)
{
	ofs << form << "|" << lemma << "|" << tag << "|" << tag.substr(0, 2) << endl;
}

void writeOutputEntries(const string &root, uint paradigm_id, const string &lemma, const string &tag, ofstream &ofs)
{
	if (tag != "-")
	{
		writeEntry(root, lemma, tag, ofs);
	}

	vector<Ending> &evec = endings[paradigm_id];

	for (uint i = 0; i < evec.size(); i++)
	{
		Ending& e = evec[i];

		string form00 = root + e.ending;
		writeEntry(form00, lemma, e.tag, ofs);

		if (e.flagNeg)
		{
			string form10 = string("ne") + form00;
			writeEntry(form10, lemma, e.tag, ofs);

			if (e.flagNej)
			{
				string form11 = string("nej") + form10;
				writeEntry(form11, lemma, e.tag, ofs);
			}
		}
		else if (e.flagNej)
		{
			string form01 = string("nej") + form00;
			writeEntry(form01, lemma, tag, ofs);
		}
	}
}

int main(int argc, char** argv)
{
	string root_file = "h2mor-cz-root.utf8";
	string ending_file = "h2mor-cz-ending.utf8";
	string output_file = "morphology-h2mor-utf8.txt";

	uint minLogCount = 2;

	if (argc > 1)
		minLogCount = atoi(argv[1]);

	fprintf(stderr, "minimum log count: %u\n", minLogCount);

	string s;
	vector<string> tokens;

	ifstream ifs_ending;
	ifs_ending.open(ending_file.c_str());

	if (ifs_ending.is_open() == false)
	{
		fprintf(stderr, "Can't open file %s!\n", root_file.c_str());
		exit(1);
	}

	int current_id = -1;

	fprintf(stderr, "reading endings...\n");

	while (std::getline(ifs_ending, s)/*MyUtils::SafeReadline(ifs_ending, s)*/)
	{
		MyUtils::Split(tokens, s, "\t ");

		Ending e;
		e.paradigm_id = MyUtils::my_atoi(tokens[0]);
		e.flagNeg = (tokens[1] == "1") ? true : false;
		e.flagNej = (tokens[2] == "1") ? true : false;
		e.ending = tokens[3];
		e.tag = processTag(tokens[4]);

		if (e.ending == "0")
			e.ending = "";

		if ((int)e.paradigm_id > current_id)
		{
			if ((int)e.paradigm_id != current_id + 1)
			{
				fprintf(stderr, "no entries for paradigm_id %u in endings file!\n", current_id + 1);
				exit(1);
			}

			endings.push_back(vector<Ending>());
			current_id++;
		}

		endings.back().push_back(e);
	}

	ifs_ending.close();

	ifstream ifs_root(root_file.c_str());

	if (ifs_root.is_open() == false)
	{
		fprintf(stderr, "Can't open file %s!\n", root_file.c_str());
		exit(1);
	}

	ofstream ofs(output_file.c_str());

	ofs << "form|lemma|tag|stag" << endl;
	ofs << "-----" << endl;

	if (ofs.is_open() == false)
	{
		fprintf(stderr, "Can't open file %s!\n", output_file.c_str());
		exit(1);
	}

	fprintf(stderr, "processing roots...\n");

	uint counter = 0;

	while (MyUtils::SafeReadline(ifs_root, s))
	{
		MyUtils::Split(tokens, s, "\t ");

		string& root = tokens[0];
		uint paradigm_id = MyUtils::my_atoi(tokens[1]);
		string &lemma = tokens[2];
		string &tag = tokens[3];
		uint log_count = MyUtils::my_atoi(tokens[4]);

		if (log_count >= minLogCount)
			writeOutputEntries(root, paradigm_id, lemma, tag, ofs);

		counter++;
		if (counter % 1000 == 0)
			fprintf(stderr, "%u\n", counter);
	}

	ofs.close();
	ifs_root.close();
}
