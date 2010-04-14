/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "MyMappedDoubleArray.hpp"
#include "ValueMapping.hpp"
#include "MyPackedArray.hpp"

namespace ngramchecker {


		void MyMappedDoubleArray::WriteToStream(ostream &ofs) const
		{
			value_mapping.writeToStream(ofs);
			mpa.WriteToStream(ofs);
		}

		MyMappedDoubleArray::MyMappedDoubleArray(vector<double> &values, uint32_t bits_per_value)
		{
			value_mapping = ValueMapping(values, bits_per_value);

			vector<uint32_t> int_vals;

			for (uint32_t i = 0; i < values.size(); i++)
			{
				int_vals.push_back(value_mapping.GetCenterID(values[i]));
			}

			mpa = MyPackedArray(int_vals);
		}

		MyMappedDoubleArray::MyMappedDoubleArray(istream &ifs):
			value_mapping(ifs), mpa(ifs)
		{}

}
