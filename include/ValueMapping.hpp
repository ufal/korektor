/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#ifndef _VALUE_MAPPING_HPP_
#define _VALUE_MAPPING_HPP_

#include "StdAfx.h"

namespace ngramchecker {


	class ValueMapping {
	private:
		vector<double> sorted_centers;
		uint num_bits_per_value;

	public:

		inline uint BitsPerValue() const
		{ 
			return num_bits_per_value; 
		}

		inline double GetDouble(uint32_t centerID) const
		{
			return sorted_centers[centerID];
		}

		uint32_t GetCenterID(double value) const;

		void writeToStream(ostream &ofs) const;

		ValueMapping(vector<double> centers);

		ValueMapping(istream &ifs);

		ValueMapping() {}

		ValueMapping(vector<double> values, const uint32_t bits_per_value);

	};

	SP_DEF(ValueMapping);
}

#endif //_VALUE_MAPPING_HPP_
