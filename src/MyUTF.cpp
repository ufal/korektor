#include "MyUTF.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <assert.h>

#include "MyPackedArray.hpp"

#include "utf8.hpp"
#include "utf16.hpp"
#include "unicode.hpp"
using namespace ufal::unilib;


#ifndef uint
#define uint uint32_t
#endif

namespace ngramchecker {

	char16_t MyUTF::tolower(char16_t ch16)
	{
		char32_t result = unicode::lowercase(ch16);
		if (char16_t(result) == result) return result;
		return ch16;
	}

	char16_t MyUTF::toupper(char16_t ch16)
	{
		char32_t result = unicode::uppercase(ch16);
		if (char16_t(result) == result) return result;
		return ch16;
	}

	string MyUTF::utf16_to_utf8(const u16string &utf16)
	{
		u32string utf32;
		utf16::decode(utf16, utf32);

		string result;
		utf8::encode(utf32, result);

		return result;
	}

	u16string MyUTF::utf8_to_utf16(const string &utf8)
	{
		u32string utf32;
		utf8::decode(utf8, utf32);

		u16string result;
		utf16::encode(utf32, result);

		return result;
	}

	bool MyUTF::is_punct(char16_t ch16)
	{
		return unicode::category(ch16) & unicode::P;
	}

	bool MyUTF::is_alphanum(char16_t ch16)
	{
		return unicode::category(ch16) & (unicode::L | unicode::Nd);
	}

	bool MyUTF::is_alpha(char16_t ch16)
	{
		return unicode::category(ch16) & unicode::L;
	}
}
