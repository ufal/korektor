#include "MyUTF.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <assert.h>

#include "unicode/ustring.h"
#include "unicode/uchar.h"

#include "MyPackedArray.hpp"


#ifndef uint
#define uint uint32_t
#endif

namespace ngramchecker {

	char16_t MyUTF::tolower(char16_t ch16)
	{
		return u_tolower(ch16);
	}

	char16_t MyUTF::toupper(char16_t ch16)
	{
		return u_toupper(ch16);
	}

	string MyUTF::utf16_to_utf8(const u16string &utf16)
	{
		vector<char> utf8_vec;
		utf8_vec.resize(utf16.size() * 2 + 1);

		int32_t destLen;
		UErrorCode errCode = U_ZERO_ERROR;
		u_strToUTF8(&utf8_vec[0], utf16.size() * 2, &destLen, (UChar*)&utf16[0], utf16.size(), &errCode);
		utf8_vec[destLen] = 0;
		return &utf8_vec[0];

	}

	u16string MyUTF::utf8_to_utf16(const string &utf8)
	{
		vector<char16_t> utf16_vec;
		utf16_vec.resize(utf8.size() + 1);

		int32_t destLen;
		UErrorCode errCode = U_ZERO_ERROR;
		u_strFromUTF8((UChar*)&utf16_vec[0], utf8.size(), &destLen, &utf8[0], utf8.size(), &errCode);
		utf16_vec[destLen] = 0;
		return &utf16_vec[0];
	}

	bool MyUTF::is_punct(char16_t ch16)
	{
		return (u_ispunct(ch16) == TRUE);
	}

	bool MyUTF::is_blank(char16_t ch16)
	{
		return (u_isblank(ch16) == TRUE);
	}

	bool MyUTF::is_control(char16_t ch16)
	{
		return (u_iscntrl(ch16) == TRUE);
	}

	bool MyUTF::is_alphanum(char16_t ch16)
	{
		return (u_isalnum(ch16) == TRUE);
	}

	bool MyUTF::is_alpha(char16_t ch16)
	{
		return (u_isalpha(ch16) == TRUE);
	}
}
