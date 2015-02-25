/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/


#ifndef _CONSTANTS_HPP_
#define _CONSTANTS_HPP_

#include "StdAfx.h"

namespace ngramchecker {

class MyConstants {
 public:
  static bool isMacOS;

  static double form_weight;
  static double lemma_weight;
  static double tag_weight;

  static uint32_t form_order;
  static uint32_t lemma_order;
  static uint32_t tag_order;

  static bool use_letter_lm;
  static bool diacritic_completion_ignore_case;

  static uint32_t default_num_suggestions;

  static uint32_t letter_decoder_order;


  static size_t universal_undefined_id;

  static uint32_t default_checking_results_cash_capacity;
  static uint32_t default_suggestions_cash_capacity;
  static uint32_t default_morphology_cash_capacity;
  static uint32_t default_lexicon_state_cash_capacity;
  static uint32_t default_ngram_cash_capacity;
  static uint32_t default_ngram_probs_cash_capacity;
  static uint32_t default_letter_ngram_cash_capacity;
  static uint32_t default_letter_ngram_probs_cash_capacity;
  static uint32_t default_lemma_signs_cash_capacity;
  static uint32_t default_stage_posibility_cash_capacity;

  static string project_home;

  static string filename_lexicon_bin;
  static string filename_morphology_bin;

  static string filename_form_zip_lm_bin;
  static string filename_lemma_zip_lm_bin;
  static string filename_tag_zip_lm_bin;
  static string filename_letter_universal_zip_lm_bin;
  static string filename_letter_names_zip_lm_bin;
  static string filename_letter_common_zip_lm_bin;

  static string filename_test_sentence;
  static string filename_chars_list;
  static string filename_utf8_list;
  static string filename_iso_list;
  static string filename_error_model_diacritics;
  static string filename_error_model_typo_manual;
  static string filename_error_model_type_auto;
  static string filename_lemma_signs;
  static string filename_keyboard_layout;
  static string filename_form_map;
  static string filename_lemma_map;
  static string filename_tag_map;
  static string filename_emmisions_bin;
  static string filename_lc_uc_letters;

  static string filename_tokenizer_config;

  static string directory_evaluation_correction;
  static string directory_evaluation_diacritic;
  static string directory_evaluation_tagger;

  static string filename_evaluation_correction_heldout_prefix;
  static string filename_evaluation_correction_test_prefix;

  static string global_lemma_signs_symbols;

  static const uint32_t default_similar_words_lookup_max_ed_dist_1round = 1;
  static const uint32_t default_similar_words_lookup_max_ed_dist_2round = 2;
  static double default_similar_words_lookup_max_cost_1round;
  static double default_similar_words_lookup_max_cost_2round;

  static double prunning_constant;

  static const uint32_t sentence_start_id = 0;
  static const uint32_t sentence_end_id = 1;
  static const uint32_t unknown_word_id = 2;
  static const uint32_t name_id = 3;

};
}
#endif //_CONSTANTS_HPP_
