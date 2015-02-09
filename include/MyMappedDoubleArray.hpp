/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

/// @file MyMappedDoubleArray.hpp
/// @class MyMappedDoubleArray MyMappedDoubleArray.hpp "MyMappedDoubleArray.hpp"
///
/// @brief Class for holding mapped array
///
/// The class @ref MyMappedDoubleArray is used to store the mapped array of original values. The mapping is done via @ref ValueMapping.

#ifndef MYMAPPEDDOUBLEARRAY_HPP_
#define MYMAPPEDDOUBLEARRAY_HPP_

#include "StdAfx.h"

#include "ValueMapping.hpp"
#include "MyPackedArray.hpp"

namespace ngramchecker {

	class MyMappedDoubleArray {
	private:
		ValueMapping value_mapping; ///< Mapping through @ref ValueMapping
		MyPackedArray mpa; ///< Mapped values

	public:
		/// @brief Get the size of the mapped values (number of values)
		///
		/// @return Size (integer)
		inline uint32_t GetSize() const
		{
			return mpa.GetSize();
		}

		/// @brief Get the mapped value at a given index
		///
		/// @param index Index
		/// @return Mapped value (double)
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
