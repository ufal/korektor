/// @file MyStaticStringArray.hpp
/// @class MyStaticStringArray MyStaticStringArray.hpp "MyStaticStringArray.hpp"
/// @brief Class for storing array of strings as data

#ifndef _MY_STATIC_STRING_ARRAY
#define _MY_STATIC_STRING_ARRAY

#include "StdAfx.h"
#include "CompIncreasingArray.hpp"

namespace ngramchecker {


class MyStaticStringArray {
  char* data; ///< Pointer to string data
  uint32_t data_size; ///< Size of the data
  CompIncreasingArray offsets;

  char pom_data[1000];

 public:

  /// @brief Get the data size
  ///
  /// @return Data size
  uint32_t GetSize()
  {
    return offsets.GetSize();
  }

  /// @brief Get the string value at a given index
  ///
  /// @param index Index value
  /// @return String value at the index
  string GetStringAt(size_t index)
  {
    CompIA_First_Last_IndexPair index_pair = offsets.GetFirstLastIndexPair(index);

    uint len = index_pair.second - index_pair.first + 1;

    memcpy(pom_data, &(data[index_pair.first]), len);

    pom_data[len] = 0;

    string ret = pom_data;
    return ret;
  }

  /// @brief Initialize the array from input stream
  ///
  /// @param ifs Input stream
  MyStaticStringArray(istream &ifs)
  {
    ifs.read((char*)&data_size, sizeof(uint32_t));

    data = new char[data_size];
    ifs.read(data, data_size);
    offsets = CompIncreasingArray(ifs);
  }

  /// @brief Initialize the array from a vector of strings
  ///
  /// @param vals Vector of strings
  MyStaticStringArray(vector<string> &vals)
  {
    uint32_t bytes_needed = 0;
    vector<uint32_t> offs;

    for (size_t i = 0; i < vals.size(); i++)
    {
      offs.push_back(bytes_needed);
      bytes_needed += vals[i].length();
    }

    data = new char[bytes_needed];

    for (size_t i = 0; i < vals.size(); i++)
    {
      memcpy(&(data[offs[i]]), vals[i].c_str(), vals[i].length());
    }

    offsets = CompIncreasingArray(offs, bytes_needed - 1);

    data_size = bytes_needed;
  }

  /// @brief Write the array to output stream
  ///
  /// @param ofs Output stream
  void WriteToStream(ostream &ofs)
  {
    ofs.write((char*)&data_size, sizeof(uint32_t));

    ofs.write(data, data_size);

    offsets.WriteToStream(ofs);
  }

  /// @brief Destructor
  ~MyStaticStringArray()
  {
    delete[] data;
  }

  /// @brief Initialize the array from another static string array
  ///
  /// @param val MyStaticStringArray
  MyStaticStringArray(const MyStaticStringArray &val)
  {
    offsets = val.offsets;
    data_size = val.data_size;

    data = new char[data_size];

    memcpy(data, val.data, data_size);
  }

  /// @brief Copy the static array through assignment operator (copy constructor)
  ///
  /// @param val MyStaticStringArray
  /// @return MyStaticStringArray
  MyStaticStringArray& operator=(const MyStaticStringArray &val)
  {
    if (this != &val)
    {
      char* new_data = new char[val.data_size];
      memcpy(new_data, val.data, val.data_size);
      data_size = val.data_size;
      offsets = val.offsets;
      delete[] data;
      data = new_data;
    }

    return *this;
  }
};

SP_DEF(MyStaticStringArray);
}

#endif
