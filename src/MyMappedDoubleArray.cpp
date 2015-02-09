/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "MyMappedDoubleArray.hpp"
#include "ValueMapping.hpp"
#include "MyPackedArray.hpp"

namespace ngramchecker {

		/// @brief Write mapped values to the output stream
		///
		/// @param ofs Output stream
		void MyMappedDoubleArray::WriteToStream(ostream &ofs) const
		{
			value_mapping.writeToStream(ofs);
			mpa.WriteToStream(ofs);
		}

		/// @brief Constructor initialization through vector of values
		///
		/// The constructor takes the vector of values and constructs the mapped values in 2 steps:
		/// (i) values are mapped to larger set of values through @reg ValueMapping and (ii) mapped
		/// values corresponding to the size of the original vector are taken from large mapped set.
		///
		/// @param values Vector of values
		/// @param bits_per_value
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

		/// @brief Constructor initialization through input stream
		///
		/// @param ifs Input stream
		MyMappedDoubleArray::MyMappedDoubleArray(istream &ifs):
			value_mapping(ifs), mpa(ifs)
		{}

}
