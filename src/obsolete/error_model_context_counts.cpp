#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>

#include <stdint.h>
#include <StdAfx.h>


#include "MyUnicodeInputStream.hpp"

using namespace std;

map<u16string, uint64_t> context_map;

void add_to_map(const u16string &s)
{
	auto fit = context_map.find(s);
	if (fit != context_map.end())
		context_map[s] = fit->second + 1;
	else
		context_map[s] = 1;

	//cerr << "added key: " << MyUtils::utf16_to_utf8(s) << ", value = " << context_map[s] << endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cerr << "2 arguments expected!" << endl;
		return -1;
	}

	ofstream ofs;
	ofs.open(argv[2]);
	if (ofs.is_open() == false)
	{
		cerr << "Can't create output file" << endl;
		return -2;
	}


	MyUTF8InputStream uis_corpus(argv[1]);
	string s;

	vector<string> toks;

	uint32_t counter = 0;
	
	while (uis_corpus.ReadLineString(s))
	{
		if (counter % 50000 == 0) cerr << counter << endl;
		counter++;
		MyUtils::Split(toks, s, " ");

		//if (counter % 500000 == 0) break;

		for (auto it = toks.begin(); it != toks.end(); it++)
		{
			size_t index = it->find('|');
			if (index == string::npos)
			{
				cerr << "error: separator | not found in: " << *it << endl;
				continue;
			}

			string tok1 = it->substr(0, index);

			//cerr << tok1 << endl;

			u16string ustr = MyUtils::utf8_to_utf16(tok1);

			bool ok = true;

			for (uint j = 0; j < ustr.length(); j++)
			{
				if (! MyUTF::is_alpha(ustr[j]) )
				{
					ok = false;
					break;
				}

				if (ustr[j] == 0)
					return -25;

				ustr[j] = MyUTF::tolower(ustr[j]);
			}

			if (ok == false)
				break;

			//cerr << "processing string: " << MyUtils::utf16_to_utf8(ustr) << endl;

			if (ustr.length() > 0)
			{
				u16string key;
				key += char16_t('+');
				key += ustr[0];
				add_to_map(key);

				if (ustr.length() > 1)
				{
					u16string key;
					key += char16_t('+');
					key += ustr[0];
					key += ustr[1];
					add_to_map(key);
				}

				for (uint i = 0; i < ustr.length(); i++)
				{
					u16string key;
					key += ustr[i];				
					add_to_map(key);

					if (i < ustr.length() - 1)
					{
						u16string key;
						key += ustr[i];
						key += ustr[i + 1];
						add_to_map(key);
					}
					else
					{
						u16string key;
						key += ustr[i];
						key += char16_t('+');
						add_to_map(key);
					}

					if (ustr.length() > 1)
					{
						if (i < ustr.length() - 2)
						{
							u16string key;
							key += ustr[i];
							key += ustr[i + 1];
							key += ustr[i + 2];
							add_to_map(key);
						}
						else if (i == ustr.length() - 2)
						{
							u16string key;
							key += ustr[i];
							key += ustr[i + 1];
							key += char16_t('+');
							add_to_map(key);
						}
					}
				}
			}

			//cerr << ustr << endl;
		}
	}

	for (auto it = context_map.begin(); it != context_map.end(); it++)
	{
		if (it->first.length() == 1)
		{
			cerr << MyUtils::utf16_to_utf8(it->first) << " " << it->second << endl;
		}

		ofs << MyUtils::utf16_to_utf8(it->first) << " " << it->second << endl;
	}


	ofs.close();

	string s1;
	cin >> s1;
}