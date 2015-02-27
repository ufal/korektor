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
#include <iostream>

#include "common.h"
#include "language_model/zip_lm.h"
#include "morphology/morphology.h"
#include "utils/utils.h"

using namespace ufal::korektor;

int main(int argc, char** argv)
{
  if (argc != 7) { cerr << "Expected 6 arguments!" << endl; return -1; }

  string lm_text_file = argv[1];
  string morphology_bin_file = argv[2];
  string morphology_words_file = argv[3];
  string factor_name = argv[4];
  unsigned order = Utils::my_atoi(argv[5]);
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
  exit(0);
}
