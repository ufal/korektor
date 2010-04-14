/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef COMPINCREASINGARRAY_HPP_
#define COMPINCREASINGARRAY_HPP_

#include "StdAfx.h"

#include "MyIncreasingArray.hpp"

namespace ngramchecker {

	typedef pair<uint32_t, uint32_t> CompIA_First_Last_IndexPair;

	class CompIncreasingArray {
	private:
		uint32_t log2_size_of_parts;
		uint32_t bit_mask;
		uint32_t num_values;
		uint32_t last_val_last_index;
		vector<MyIncreasingArrayP> mia_vec;


	public:

		inline uint32_t GetSize() const
		{
			return num_values;
		}

		//TODO: optimize this instead of / and % use >> and &
		inline uint32_t GetValueAt(uint32_t i) const
		{
			return mia_vec[i >> log2_size_of_parts]->GetValueAt(i & bit_mask);
		}

		inline CompIA_First_Last_IndexPair GetFirstLastIndexPair(uint32_t i) const
		{

			uint32_t first = GetValueAt(i);

			uint32_t last;
			if (i < GetSize() - 1)
			{
				last = GetValueAt(i + 1);
				if (last == 0)
				{
					return make_pair(1, 0);
				}
				last--;
			}
			else
				last = last_val_last_index;

			assert(last <= last_val_last_index);

			return make_pair(first, last);
		}

		void WriteToStream(ostream &ofs) const;
		void WriteToStreamNew(ostream &ofs) const;

		CompIncreasingArray() {}

		CompIncreasingArray(istream &ifs);

		CompIncreasingArray(vector<uint32_t> &val, uint32_t _last_val_last_index);
	};

	SP_DEF(CompIncreasingArray);
}

#endif /* COMPINCREASINGARRAY_HPP_ */
