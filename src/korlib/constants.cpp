// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "common.h"
#include "constants.h"
#include "error_model.h"

namespace ufal {
namespace korektor {

bool MyConstants::isMacOS = false;

double MyConstants::form_weight = 0.312;
double MyConstants::lemma_weight = 0.141;
double MyConstants::tag_weight = 0.547;

uint32_t MyConstants::form_order = 3;
uint32_t MyConstants::lemma_order = 3;
uint32_t MyConstants::tag_order = 3;

bool MyConstants::use_letter_lm = false;
bool MyConstants::diacritic_completion_ignore_case = false;

uint32_t MyConstants::default_num_suggestions = 5;

uint32_t MyConstants::letter_decoder_order = 7;

size_t MyConstants::universal_undefined_id = std::numeric_limits<uint32_t>::max();

uint32_t MyConstants::default_checking_results_cash_capacity = 5000;
uint32_t MyConstants::default_suggestions_cash_capacity = 5000;
uint32_t MyConstants::default_morphology_cash_capacity = 5000;
uint32_t MyConstants::default_lexicon_state_cash_capacity = 15000;
uint32_t MyConstants::default_ngram_cash_capacity = 60000;
uint32_t MyConstants::default_ngram_probs_cash_capacity = 100000;
uint32_t MyConstants::default_letter_ngram_cash_capacity = 15000;
uint32_t MyConstants::default_letter_ngram_probs_cash_capacity = 40000;
uint32_t MyConstants::default_lemma_signs_cash_capacity = 40000;
uint32_t MyConstants::default_stage_posibility_cash_capacity = 10000;

string MyConstants::global_lemma_signs_symbols = "aehlnstvx";

//uint32_t MyConstants::default_similar_words_lookup_max_ed_dist_1round = 1;
//uint32_t MyConstants::default_similar_words_lookup_max_ed_dist_2round = 2;
double MyConstants::default_similar_words_lookup_max_cost_1round = 20;
double MyConstants::default_similar_words_lookup_max_cost_2round = 40;

double MyConstants::prunning_constant = 6;

const float ErrorModel::impossible_cost = 1000;
const float ErrorModel::max_cost = 100;
const float ErrorModel::name_cost = 70;
const float ErrorModel::unknown_cost = 100;

string MyConstants::project_home;

string MyConstants::filename_lexicon_bin;
string MyConstants::filename_morphology_bin;

string MyConstants::filename_form_zip_lm_bin;
string MyConstants::filename_lemma_zip_lm_bin;
string MyConstants::filename_tag_zip_lm_bin;
string MyConstants::filename_letter_universal_zip_lm_bin;
string MyConstants::filename_letter_names_zip_lm_bin;
string MyConstants::filename_letter_common_zip_lm_bin;

string MyConstants::filename_test_sentence;
string MyConstants::filename_chars_list;
string MyConstants::filename_utf8_list;
string MyConstants::filename_iso_list;
string MyConstants::filename_error_model_diacritics;
string MyConstants::filename_error_model_typo_manual;
string MyConstants::filename_error_model_type_auto;
string MyConstants::filename_lemma_signs;
string MyConstants::filename_keyboard_layout;
string MyConstants::filename_form_map;
string MyConstants::filename_lemma_map;
string MyConstants::filename_tag_map;
string MyConstants::filename_emmisions_bin;
string MyConstants::filename_lc_uc_letters;

string MyConstants::directory_evaluation_correction;
string MyConstants::directory_evaluation_diacritic;
string MyConstants::directory_evaluation_tagger;



string MyConstants::filename_evaluation_correction_heldout_prefix;
string MyConstants::filename_evaluation_correction_test_prefix;

} // namespace korektor
} // namespace ufal
