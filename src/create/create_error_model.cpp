// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <cstring>

#include "common.h"
#include "create_error_model/create_error_hierarchy.h"
#include "create_error_model/error_hierarchy.h"
#include "create_error_model/estimate_error_model.h"
#include "create_error_model/get_error_signature.h"
#include "error_model/error_model_basic.h"
#include "utils/io.h"
#include "utils/options.h"
#include "utils/parse.h"
#include "utils/utf.h"
#include "version/version.h"

using namespace ufal::korektor;

std::unordered_map<u16string, hierarchy_nodeP> hierarchy_node::hierarchy_map;
hierarchy_nodeP hierarchy_node::root;

int main(int argc, char** argv)
{
  iostream::sync_with_stdio(false);

  Options::Map options;
  if (!Options::Parse({{"train", Options::Value::none},
                      {"binarize", Options::Value::none},
                      {"version", Options::Value::none},
                      {"help", Options::Value::none}}, argc, argv, options) ||
      options.count("help") ||
      (options.count("binarize") && options.count("train")) ||
      (!options.count("version") && !options.count("binarize") && !options.count("train")) ||
      (!options.count("version") && options.count("binarize") && argc != 3) ||
      (!options.count("version") && options.count("train") && argc != 5))
    runtime_failure("Usage: " << argv[0] << " -binarize text_error_model out_binary_error_model\n"
                    "    or " << argv[0] << " -train error_hierarchy spelling_errors word_list out_text_error_model\n"
                    "Options: --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  if (options.count("binarize"))
  {
    ErrorModelBasic::CreateBinaryFormFromTextForm(argv[1], argv[2]);
  }
  else /*if (options.count("train"))*/
  {
    cerr << "reading error hierarchy..." << endl;

    ifstream ifs_hierarchy;
    ifs_hierarchy.open(argv[1]);
    if (!ifs_hierarchy.is_open())
      runtime_failure("Cannot open file '" << argv[1] << "'!");

    hierarchy_node::ReadHierarchy(ifs_hierarchy);

    ifs_hierarchy.close();

    cerr << "hierarchy read!" << endl;

    //hierarchy_node::print_hierarchy_rec(hierarchy_node::root, 0, cerr);

    string error_line;

    ifstream ifs_errors(argv[2]);
    if (!ifs_errors.is_open())
      runtime_failure("Cannot open errors file '" << argv[2] << "'!");

    while (IO::ReadLine(ifs_errors, error_line))
    {
      if (error_line.empty() || error_line.substr(0, 2) == "//")
        continue;

      vector<string> toks;
      IO::Split(error_line, " \t", toks);

      if (toks.size() != 2)
        runtime_failure("Not two columns on line '" << error_line << "' in file '" << argv[2] << "'!");

      u16string signature;
      if (GetErrorSignature(UTF::UTF8To16(toks[0]), UTF::UTF8To16(toks[1]), signature))
      {
        if (hierarchy_node::ContainsNode(signature))
        {
          hierarchy_nodeP hnode = hierarchy_node::GetNode(signature);
          hnode->error_count++;
        }
      }
      else
      {
        cerr << "error not recognized: " << error_line << endl;
      }
    }

    ifstream ifs_context(argv[3]);
    if (!ifs_context.is_open())
      runtime_failure("Cannot open context file '" << argv[3] << "'!");

    string s;
    vector<string> toks;

    unordered_map<u16string, uint32_t> context_map;

    while(IO::ReadLine(ifs_context, s))
    {
      IO::Split(s, ' ', toks);

      if (s.empty()) continue;

      if (toks.size() != 2)
        runtime_failure("Not two columns on line '" << s << "' in file '" << argv[3] << "'!");

      u16string key = UTF::UTF8To16(toks[0]);
      uint32_t count = Parse::Int(toks[1], "context count");

      for (unsigned i = 0; i < key.length(); i++)
        if (key[i] == char16_t('+'))
          key[i] = char16_t(' ');

      if (context_map.find(key) != context_map.end())
      {
        cerr << "key already found in the map!!!!!!" << UTF::UTF16To8(key) << "!!!" << endl;
        context_map[key] += count;
      }
      else
      {
        context_map[key] = count;
      }
    }

    EstimateErrorModel eem = EstimateErrorModel(hierarchy_node::root, context_map);
    eem.Estimate();

    ofstream ofs_errmodel_txt;
    ofs_errmodel_txt.open(argv[4]);
    if (!ofs_errmodel_txt.is_open())
    {
      cerr << "Can't create " << argv[4] << endl;
      return -10;
    }

    vector<pair<u16string, ErrorModelOutput>> out_vec;

    hierarchy_node::output_result_rec(hierarchy_node::root, 0, 0, 1.0f, hierarchy_node::root->signature, out_vec);

    ofs_errmodel_txt << "case\t0\t2.0" << endl;

    hierarchy_nodeP subs = hierarchy_node::GetNode(UTF::UTF8To16("substitutions"));
    ErrorModelOutput emo_subs = ErrorModelOutput(1, subs->error_prob);

    ofs_errmodel_txt << "substitutions\t" << emo_subs.edit_dist << "\t" << emo_subs.cost << endl;

    hierarchy_nodeP inserts = hierarchy_node::GetNode(UTF::UTF8To16("insertions"));
    ErrorModelOutput emo_inserts = ErrorModelOutput(1, inserts->error_prob);

    ofs_errmodel_txt << "insertions\t" << emo_inserts.edit_dist << "\t" << emo_inserts.cost << endl;

    hierarchy_nodeP deletes = hierarchy_node::GetNode(UTF::UTF8To16("deletions"));
    ErrorModelOutput emo_deletes = ErrorModelOutput(1, deletes->error_prob);

    ofs_errmodel_txt << "deletions\t" << emo_deletes.edit_dist << "\t" << emo_deletes.cost << endl;

    hierarchy_nodeP swaps = hierarchy_node::GetNode(UTF::UTF8To16("swaps"));
    ErrorModelOutput emo_swaps = ErrorModelOutput(1, swaps->error_prob);

    ofs_errmodel_txt << "swaps\t" << emo_swaps.edit_dist << "\t" << emo_swaps.cost << endl;

    for (auto it = out_vec.begin(); it != out_vec.end(); it++)
    {
      ofs_errmodel_txt << UTF::UTF16To8(it->first) << "\t" << it->second.edit_dist << "\t" << it->second.cost << endl;
    }

    ofs_errmodel_txt.close();

  }

  cerr << "Error model successfully created." << endl;
  return 0;
}
