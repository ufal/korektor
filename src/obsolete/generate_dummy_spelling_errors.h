#include "utils.h"

void GenerateDummySpellingErrors(const string &instructions_file, const string &output_file)
{
  ofstream ofs_errs;
  ofs_errs.open(output_file.c_str());
  FATAL_CONDITION(ofs_errs.is_open(), "");

  MyUTF8InputStream er_gen_stream(instructions_file);

  string s;

  while (er_gen_stream.ReadLineString(s))
  {
    vector<string> parts;

    MyUtils::Split(parts, s, " ");
    u16string signature = MyUtils::utf8_to_utf16(parts[0]);
    uint number = MyUtils::my_atoi(parts[1]);

    if (hierarchy_node::ContainsNode(signature) == false)
    {
      cerr << MyUtils::utf16_to_utf8(signature) << endl;
      continue;
    }
    hierarchy_nodeP node = hierarchy_node::GetNode(signature);

    ofs_errs << "//===" << MyUtils::utf16_to_utf8(signature) << endl;

    for (uint i = 0; i < number; i++)
    {
      u16string misspelled;
      u16string correct;

      node->generate_spelling_error(misspelled, correct);


      ofs_errs << MyUtils::utf16_to_utf8(misspelled) << " " << MyUtils::utf16_to_utf8(correct) << endl;
    }
  }

  ofs_errs.close();
}
