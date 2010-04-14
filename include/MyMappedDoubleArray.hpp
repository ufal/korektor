/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef MYMAPPEDDOUBLEARRAY_HPP_
#define MYMAPPEDDOUBLEARRAY_HPP_

#include "StdAfx.h"

#include "ValueMapping.hpp"
#include "MyPackedArray.hpp"

namespace ngramchecker {

	class MyMappedDoubleArray {
	private:
		ValueMapping value_mapping;
		MyPackedArray mpa;

	public:
		inline uint32_t GetSize() const
		{
			return mpa.GetSize();
		}

		inline double GetValueAt(uint32_t index) const
		{
			return value_mapping.GetDouble(mpa.GetValueAt(index));
		}

		void WriteToStream(ostream &ofs) const;

		MyMappedDoubleArray(vector<double> &values, uint32_t bits_per_value);

		MyMappedDoubleArray(istream &ifs);
	};

	SP_DEF(MyMappedDoubleArray);
}

#endif /* MYMAPPEDDOUBLEARRAY_HPP_ */
