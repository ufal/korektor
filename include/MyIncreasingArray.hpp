/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef MYINCREASINGARRAY_HPP_
#define MYINCREASINGARRAY_HPP_

#include "StdAfx.h"

#include "MyPackedArray.hpp"

namespace ngramchecker {

	class MyIncreasingArray {
	private:
		uint32_t multiplier;
		int32_t value_shift;
		MyPackedArray shifted_offsets;

	public:

		inline uint32_t GetSize() const
		{
			return shifted_offsets.GetSize();
		}

		inline uint32_t GetValueAt(uint32_t i) const
		{
			return ((i * multiplier) >> 10) + shifted_offsets.GetValueAt(i) + value_shift;
		}

		MyIncreasingArray(istream &ifs);

		void WriteToStream(ostream &ofs) const;

		MyIncreasingArray(vector<uint32_t> &vec);

	};

	SP_DEF(MyIncreasingArray);
}

#endif /* MYINCREASINGARRAY_HPP_ */
