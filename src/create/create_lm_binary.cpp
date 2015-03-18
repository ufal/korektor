// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file create_lm_binary.cpp
/// @brief Tool for creating language model (LM) binaries
///
/// Usage: ./create_lm_binary [ARPA language model] [binary morphology] [binary vocabulary] [factor name] [language model order] [output binary language model]

#include <fstream>

#include "common.h"
#include "language_model/zip_lm.h"
#include "morphology/morphology.h"
#include "utils/options.h"
#include "utils/parse.h"
#include "version/version.h"

using namespace ufal::korektor;

int main(int argc, char** argv)
{
  iostream::sync_with_stdio(false);

  Options::Map options;
  if (!Options::Parse({{"version", Options::Value::none},
                      {"help", Options::Value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc != 7 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " ARPA_lm morphology vocabulary factor_name model_order out_lm\n"
                    "Options: --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  string lm_text_file = argv[1];
  string morphology_bin_file = argv[2];
  string morphology_words_file = argv[3];
  string factor_name = argv[4];
  unsigned order = Parse::Int(argv[5], "language model order");
  string out_file = argv[6];
  double not_in_lm_cost = 15.0;

  ifstream ifs;
  ifs.open(morphology_bin_file.c_str(), ios::binary);
  assert(ifs.is_open());
  MorphologyP morphology = MorphologyP(new Morphology(ifs));
  morphology->initMorphoWordLists(morphology_words_file);

  ZipLMP lm = ZipLM::createFromTextFile(lm_text_file, morphology, factor_name, order, not_in_lm_cost);
  lm->SaveInBinaryForm(out_file);

  cerr << "saved!" << endl;
  return 0;
}
