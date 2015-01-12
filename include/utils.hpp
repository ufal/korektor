/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef MICHALISEKSPELL_UTILS_HPP_
#define MICHALISEKSPELL_UTILS_HPP_

#include "StdAfx.h"
#include "MyUTF.hpp"

#define FATAL_CONDITION(expr, error_text) if (!(expr)) { std::cerr << "FATAL ERROR! The condition ( " << #expr << " ) failed. " << error_text << std::endl; string dummy_string; std::cin >> dummy_string; exit(-1); } 
#define WARNING(expr, error_text) if (!(expr)) { std::cerr << "FATAL ERROR! The condition ( " << #expr << " ) failed. " << error_text << std::endl; } 
#define FATAL(error_text) std::cerr << "FATAL ERROR: " << error_text << endl; string dummy_string; std::cin >> dummy_string; exit(-1);
namespace ngramchecker {

	class MyUtils {
	public:
		
		static void HashCombine(size_t &seed, uint value)
		{
			seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
		}

		//utility for writing a string to a binary stream
		static void WriteString(ostream &ofs, const string &s)
		{
			FATAL_CONDITION(uint16_t(s.length()) == s.length(), "");
			uint16_t len = s.length();
			ofs.write((char*)&len, sizeof(uint16_t));
			ofs.write(s.data(), sizeof(char) * len);
		}

		/// @brief utility for reading a string value from a binary stream
		static string ReadString(istream &ifs)
		{
			uint16_t len;
			ifs.read((char*)&len, sizeof(uint16_t));
			char* chars = new char[len + 1];
			ifs.read(chars, len);
			chars[len] = 0;
			string ret = chars;
			delete[] chars;

			return ret;
		}

		static uint32_t Read_uint32_t(istream &is)
		{
			uint32_t ret;
			is.read((char*)&ret, sizeof(uint32_t));
			return ret;
		}

		//random number in a range
		static double RandomNumber(double min, double max);
		
		//random number in a range
		static int randomR(int lowest, int range);

		static double minus_log_10_rand01();

		//minimal number of bits needed for representing the given unsigned int value
		static uint32_t BitsNeeded(uint32_t value)
		{
			uint32_t ret = 0;
			uint32_t pom = 1;
			while (pom <= value)
			{
				ret++;
				pom = pom << 1;
			}

			return ret;
		}

		static bool CaseInsensitiveIsEqual(const u16string &str1, const u16string &str2)
		{
			if (str1.length() != str2.length())
				return false;

			for (uint i = 0; i < str1.length(); i++)
			{
				if (MyUTF::tolower(str1[i]) != MyUTF::tolower(str1[i]))
					return false;
			}

			return true;
		}

		//outputting a vector of strings into cerr
		static void cerr_vector_string(const vector<string> &vec);

		static string GetTimeString();

		static bool StrToBool(string str);

		static double StrToDouble(string str);

		static uint32_t StrToInt(string str);

		static string BoolToString(bool value);

		static string utf16_to_utf8(const u16string &ustr)
		{
			return MyUTF::utf16_to_utf8(ustr);
		}

		static u16string utf8_to_utf16(const string &str)
		{
			return MyUTF::utf8_to_utf16(str);
		}

		static bool IsUpperCase(char16_t uchar)
		{
			return MyUTF::tolower(uchar) != uchar;
		}

		static bool IsLowerCase(char16_t uchar)
		{
			return MyUTF::toupper(uchar) != uchar;
		}

		static char16_t ToLower(char16_t uchar)
		{
			return MyUTF::tolower(uchar);
		}

		static char16_t ToUpper(char16_t uchar)
		{
			return MyUTF::toupper(uchar);
		}

		static bool ContainsLetter(const u16string &ustr);

		//gets the capitalization type for the given string - i.e. whether it's all lowercase, all upercase, first letter capital, or something crazy
		static capitalization_type Get_u16string_capitalization_type(const u16string &ustr);

		//wrapper around std::getline - eventually removes '\r' from the end - i.e. when a text file created on Windows is being opened on Linux
		static bool SafeReadline(istream &istr, string &str)
		{
			if (!std::getline(istr, str))
				return false;
			else
			{
				if (str.length() > 0 && str[str.length() - 1] == '\r')
				{
					str.erase(str.length() - 1);
				}
				return true;
			}
		}

		//split the string in s into toks, characters contained in delims form a set of delimiters (i.e. they are not included into toks)
		static void Split(vector<string> &toks, const string &s, const string &delims)
		{
			toks.clear();

			string::const_iterator segment_begin = s.begin();
			string::const_iterator current = s.begin();
			string::const_iterator string_end = s.end();

			while (true)
			{
				if (current == string_end || delims.find(*current) != string::npos || *current == '\r')
				{
					if (segment_begin != current)
						toks.push_back(string(segment_begin, current));

					if (current == string_end || *current == '\r')
						break;

					segment_begin = current + 1;
				}

				current++;
			}

		}

		static float my_atof(const string &str)
		{
			float ret = (float)std::atof(str.c_str());

			//if (str.empty() || (ret == 0.0f && str[0] != '0'))
			//	throw std::bad_cast("error in my_atof: value cannot be converted to float!");

			return ret;
		}

		static int my_atoi(const string &str)
		{
			int ret = std::atoi(str.c_str());

			//if (str.empty() || ret == 0 && str[0] != '0')
			//	throw std::bad_cast("error in my_atoi: value cannot be converted to int!");

			return ret;
		}

		static string my_itoa(int val)
		{
			static char buffer[200];
			sprintf(buffer, "%d", val);
			return string(buffer);
		}

	};

	//hashing function for u16string
	struct u16string_hash
	{
		size_t operator()(const u16string &val) const
		{
			size_t seed = 0;
			for (size_t i = 0; i < val.length(); i++)
			{
				MyUtils::HashCombine(seed, val[i]);
			}

			return seed;
		}
	};

}

#endif /* MICHALISEKSPELL_UTILS_HPP_ */
