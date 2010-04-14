#ifndef _USTRING_REPLACING_HPP
#define _USTRING_REPLACING_HPP

#include "StdAfx.h"

namespace ngramchecker {

	class u16stringReplacing {
		u16string ustring;
		int offset;
		uint first_allowed_start;
	public:
		u16stringReplacing(const u16string &_ustring): ustring(_ustring), offset(0), first_allowed_start(0) {}

		void Replace(uint start, uint length, const u16string &replacement)
		{
			assert(start >= first_allowed_start);
			ustring.replace(start + offset, length, replacement);
			offset += replacement.length() - length;
			first_allowed_start = start + length;
		}

		u16string GetResult()
		{
			return ustring;
		}
	};

}

#endif //_USTRING_REPLACING_HPP