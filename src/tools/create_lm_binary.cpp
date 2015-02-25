/// @file create_lm_binary.cpp
/// @brief Tool for creating language model (LM) binaries
///
/// Usage: ./create_lm_binary [ARPA language model] [binary morphology] [binary vocabulary] [factor name] [language model order] [output binary language model]

#include "common.h"
#include "korlib/morphology.h"
#include "korlib/zip_lm.h"

int main(int argc, char** argv)
{
  if (argc != 7) { cerr << "Expected 6 arguments!" << endl; return -1; }

  string lm_text_file = argv[1];
  string morphology_bin_file = argv[2];
  string morphology_words_file = argv[3];
  string factor_name = argv[4];
  uint order = MyUtils::my_atoi(argv[5]);
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
