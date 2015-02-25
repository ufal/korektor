#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <xstring>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include "MyPackedArray.hpp"

#include "StdAfx.h"

#include "MyUTF.hpp"

using namespace std;

#define uchar unsigned char
#define uint uint32_t

#include <unicode/uchar.h>
#include <unicode/utf.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/ustring.h>


int main(int argc, char** argv)
{
  u_setDataDirectory("C:\\");
  vector<uint16_t> uc_to_lc;
  vector<uint16_t> lc_to_uc;
  vector<uint> char_types;


  uc_to_lc.reserve(0xD800);
  lc_to_uc.reserve(0xD800);

  uint num_alnum = 0;
  uint num_punct = 0;
  uint num_cntrl = 0;
  uint num_blank = 0;
  uint num_else = 0;

  for (uint i = 0; i < 0xD800; i++)
  {

    uint out = (uint)u_toupper((UChar32)i);
    assert(out <= 0xFFFF);
    //cerr << "lc_to_uc: " << i << " --> " << out << endl;
    lc_to_uc.push_back(out);

    out = (uint)u_tolower((UChar32)i);
    assert(out <= 0xFFFF);
    //cerr << "uc_to_lc: " << i << " --> " << out << endl;
    uc_to_lc.push_back(out);

    if (u_ispunct((UChar)i))
    {
      char_types.push_back((uint)MyUTF::punctuation);
      num_punct++;
    }
    else if (u_isalnum((UChar)i))
    {
      char_types.push_back((uint)MyUTF::alphanum);
      num_alnum++;
    }
    else if (u_iscntrl((UChar)i))
    {
      char_types.push_back((uint)MyUTF::control);
      num_cntrl++;
    }
    else if (u_isblank((UChar)i))
    {
      char_types.push_back((uint)MyUTF::blank);
      num_blank++;
    }
    else
    {
      char_types.push_back((uint)MyUTF::something_else);
      num_else++;
    }
  }

  cerr << "num_punct = " << num_punct << ", num_alnum = " << num_alnum << ", num_cntrl = " << num_cntrl << ", num_blank = " << num_blank << ", num_else = " << num_else << endl;

  ofstream ofs_uc_to_lc;
  ofs_uc_to_lc.open("uc_to_lc.dat", ios::binary);
  ofstream ofs_lc_to_uc;
  ofs_lc_to_uc.open("lc_to_uc.dat", ios::binary);

  ofs_uc_to_lc.write((char*)uc_to_lc.data(), uc_to_lc.size() * sizeof(uint16_t));
  ofs_uc_to_lc.close();

  ofs_lc_to_uc.write((char*)lc_to_uc.data(), lc_to_uc.size() * sizeof(uint16_t));
  ofs_lc_to_uc.close();

  MyPackedArray mpa = MyPackedArray(char_types);

  ofstream ofs_cht;
  ofs_cht.open("char_types.dat", ios::binary);

  mpa.WriteToStream(ofs_cht);

  for (uint i = 0; i < 400; i++)
  {
    cerr << i << " -> " << mpa.GetValueAt(i) << " - " << char_types[i] << endl;

  }

  cerr << "alphanum: " << MyUTF::alphanum << endl;
  cerr << "punctuation: " << MyUTF::punctuation << endl;
  cerr << "blank: " << MyUTF::blank << endl;
  cerr << "control: " << MyUTF::control << endl;
  cerr << "something else: " << MyUTF::something_else << endl;

  string ble;
  cin >> ble;

  ofs_cht.close();



  return -123456;
  //MyUTF::init_mapping();

  uint i = 0;
  cerr << "0: " << i << endl;
  i += 128;
  cerr << "1: " << i << endl;
  i += 1 << 11;
  cerr << "2: " << i << endl;
  i += 1 << 16;
  cerr << "3: " << i << endl;
  i += 1 << 21;
  cerr << "4: " << i << endl;
  i += 1 << 26;
  cerr << "5: " << i << endl;

  ifstream ifs;
  ifs.open(argv[1]);
  assert(ifs.is_open());

  ofstream ofs1;
  ofs1.open(argv[2]);
  assert(ofs1.is_open());

  ofstream ofs2;
  ofs2.open(argv[3], ios::binary);
  assert(ofs2.is_open());

  ifstream ifs2;
  ifs2.open(argv[4]);
  assert(ifs2.is_open() || ios::binary);

  while (ifs2.eof() == false)
  {
    uint16_t byte2;
    ifs2.read((char*)&byte2, sizeof(uint16_t));
    cerr << byte2 << endl;
  }

  string s;
  while (MyUtils::SafeReadline(ifs, s))
  {
    cerr << s << endl;
    u16string u16str = MyUTF::utf8_to_utf16(s);

    for (auto it = u16str.begin(); it != u16str.end(); it++)
    {
      *it = MyUTF::toupper(*it);
    }

    uint16_t start_val = 65279;
    ofs2.write((char*)&start_val, sizeof(uint16_t));
    for (auto it = u16str.begin(); it != u16str.end(); it++)
    {
      char16_t ch = *it;
      ofs2.write((char*)&ch, sizeof(char16_t));
    }

    char16_t endl1 = '\r';
    char16_t endl2 = '\n';

    ofs2.write((char*)&endl1, sizeof(char16_t));
    ofs2.write((char*)&endl2, sizeof(char16_t));

    //ofs2 << '\r' << 0 << 'n' << 0;

    string conv_back = MyUTF::utf16_to_utf8(u16str);



    ofs1 << conv_back << endl;
  }

  cin >> s;

}
