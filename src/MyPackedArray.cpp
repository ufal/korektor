/*
Copyright (c) 2012, Charles University in Prague 
All rights reserved.
*/

#include "MyPackedArray.hpp"
#include "utils.hpp"

namespace ngramchecker {

			void MyPackedArray::PrintBits() const
			{
				for (uint32_t i = 0; i < num_bytes; i++)
				{
					for (uint32_t j = 0; j < 8; j++)
					{
						cout << (1 & (data[i] >> j));
					}
					cout << endl;
				}
			}


			uint32_t MyPackedArray::GetSize() const
			{
				return num_values;
			}

			uint32_t MyPackedArray::GetNumberOfBytes() const
			{
				return num_bytes;
			}

			uint32_t MyPackedArray::GetBitsPerValue() const
			{
				return bits_per_value;
			}

			MyPackedArray::~MyPackedArray()
			{
				if (data != NULL)
					delete[] data;
			}

			MyPackedArray::MyPackedArray(const MyPackedArray& val)
			{
				bits_per_value = val.bits_per_value;
				num_bytes = val.num_bytes;
				num_values = val.num_values;
				output_mask = val.output_mask;

				data = new unsigned char[num_bytes];

				memcpy(data, val.data, val.num_bytes * sizeof(unsigned char));

				//cerr << "MyPackedArray: copy constructor\n";
			}

			MyPackedArray& MyPackedArray::operator=(const MyPackedArray& val)
			{
				if (this != &val)
				{
					unsigned char* new_data = new unsigned char[val.num_bytes];

					memcpy(new_data, val.data, val.num_bytes * sizeof(unsigned char));

					bits_per_value = val.bits_per_value;
					num_bytes = val.num_bytes;
					num_values = val.num_values;
					output_mask = val.output_mask;

					if (data != NULL)
						delete[] data;
					data = new_data;

				}

				//cerr << "MyPackedArray: assignemt operator\n";
				return *this;
			}

			MyPackedArray::MyPackedArray(istream &ifs)
			{
				string check_string = MyUtils::ReadString(ifs);

				FATAL_CONDITION(check_string == "MPA", check_string);

				ifs.read((char*)&bits_per_value, sizeof(uint32_t));
				ifs.read((char*)&num_bytes, sizeof(uint32_t));
				ifs.read((char*)&num_values, sizeof(uint32_t));
				ifs.read((char*)&output_mask, sizeof(uint32_t));

				data = new unsigned char[num_bytes];

				ifs.read((char*)data, sizeof(unsigned char) * num_bytes);

			}

			void MyPackedArray::WriteToStream(ostream &ofs) const
			{
				MyUtils::WriteString(ofs, "MPA");
				ofs.write((char*)&bits_per_value, sizeof(uint32_t));
				ofs.write((char*)&num_bytes, sizeof(uint32_t));
				ofs.write((char*)&num_values, sizeof(uint32_t));

				ofs.write((char*)&output_mask, sizeof(uint32_t));
				ofs.write((char*)data, sizeof(unsigned char) * num_bytes);

			}

			MyPackedArray::MyPackedArray(const vector<uint32_t> &values)
			{
				uint32_t max = 0;
				for (uint32_t i = 0; i < values.size(); i++)
				{
					if (values[i] > max) max = values[i];
				}

				uint32_t num_bits = 0;
				uint32_t pom = 1;
				while (max + 1 > pom)
				{
					pom = pom << 1;
					num_bits++;
				}

				num_bytes = (num_bits * values.size() + 7) / 8;

				bits_per_value = num_bits;
				num_values = values.size();

				output_mask = 0;
				for (uint32_t i = 0; i < num_bits; i++)
				{
					output_mask = output_mask << 1;
					output_mask++;
				}

				data = new unsigned char[num_bytes];

				for (uint32_t i = 0; i < num_bytes; i++)
					data[i] = 0;

				uint32_t byte_pointer;
				uint32_t bit_pointer;
				ulong64 curr_value;
				unsigned char* ukaz;
				for (uint32_t i = 0; i < values.size(); i++)
				{
					int bites_left = bits_per_value;
					byte_pointer = (i * num_bits) >> 3;
					bit_pointer = (i * num_bits) % 8;

					curr_value = values[i];

					curr_value = curr_value << bit_pointer;
					ukaz = (unsigned char*)&curr_value;
					bites_left -= 8 - bit_pointer;
					data[byte_pointer] = data[byte_pointer] | ukaz[0];

					uint32_t j = 1;
					while (bites_left > 0)
					{
						data[byte_pointer + j] = data[byte_pointer + j] | ukaz[j];
						bites_left -= 8;
						j++;
					}
				}

#ifdef _DEBUG
				for (uint32_t i = 0; i < values.size(); i++)
				{
					uint32_t val = GetValueAt(i);
					if (val != values[i])
					{
						for (uint j = 0; j < values.size(); j++)
							cerr << "values[" << j << "] = " << values[j] << endl;
						cerr << "val == " << val << ", values[" << i << "] == " << values[i] << endl;
					}
					assert(val == values[i]);
				}
#endif
				//PrintBits();

			}
}
