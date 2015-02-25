// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "zip_lm.h"
#include "my_packed_array.h"
#include "my_mapped_double_array.h"
#include "comp_increasing_array.h"
#include "ngram.h"
#include "utils.h"
#include "morphology.h"

namespace ngramchecker {

int ZipLM::search_for_id(uint32_t _order, uint32_t word_id, int start_offset, int end_offset)
{
  if (end_offset < start_offset)
  {
    //cerr << "end_offset = " << end_offset << ", start_offset = " << start_offset << endl;
    return -1;
  }

  if (_order == 0)
  {
    if (word_id > max_unigram_id)
      return -1;
    else
      return word_id;
  }

  uint32_t index;
  uint32_t val;

  MyPackedArrayP id_arr = ids[_order];

  //FATAL_CONDITION(end_offset > start_offset, "");

  while (end_offset - start_offset > 1)
  {
    index = (start_offset + end_offset) / 2;
    val = id_arr->GetValueAt(index);
    if (val < word_id)
    {
      start_offset = index;
    }
    else if (val > word_id)
    {
      end_offset = index;
    }
    else
    {
      start_offset = index;
      break;
    }
  }

  if (id_arr->GetValueAt(start_offset) == word_id)
    return start_offset;
  else if (id_arr->GetValueAt(end_offset) == word_id)
    return end_offset;
  else
    return -1;
}

void ZipLM::get_lm_tuple(uint32_t _order, uint32_t _offset, LM_tuple &ret)
{

  double prob = probs[_order]->GetValueAt(_offset);
  double bow;

  if (_order < lm_order - 1)
    bow = bows[_order]->GetValueAt(_offset);
  else
    bow = 0;

  uint32_t word_id;
  if (_order > 0)
    word_id = ids[_order]->GetValueAt(_offset);
  else
    word_id = _offset;

  if (_order < lm_order - 1)
  {
    CompIA_First_Last_IndexPair offset_pair = offsets[_order]->GetFirstLastIndexPair(_offset);

    //ret.word_id = word_id;
    ret.nlevel_offset = offset_pair.first;
    ret.nlevel_entries = offset_pair.second - offset_pair.first + 1;
    ret.prob = prob;
    ret.bow = bow;
    return;
  }
  else
  {
    //ret.word_id = word_id;
    ret.nlevel_offset = 1;
    ret.nlevel_entries = 0;
    ret.prob = prob;
    ret.bow = bow;
    return;
  }
}

bool ZipLM::getFirstLevelTuple(uint word_id, LM_tuple &ret)
{
  if (word_id > max_unigram_id)
    return false;

  get_lm_tuple(0, word_id, ret);

  return true;
}

bool ZipLM::GetTuple(uint level, uint word_id, uint offset, uint num_entries, LM_tuple &lm_tuple)
{
  int ret_offset = search_for_id(level, word_id, offset, offset + num_entries - 1);

  if (ret_offset == -1)
    return false;

  get_lm_tuple(level, ret_offset, lm_tuple);

  return true;
}

string ZipLM::GetFilename()
{
  return filename;
}

double ZipLM::GetWordNotInLMCost()
{
  return not_in_lm_cost;
}


void ZipLM::GetNGramForNGramKey(NGram &ngram_key, NGram &ngram_ret)
{
  assert(ngram_key.order <= lm_order);

  uint32_t input_order = ngram_key.order;

  int start_offset = 0;
  int end_offset = 0;

  uint ret_order = 0;

  LM_tuple lm_tup;

  for (uint32_t order = 0; order < input_order; order++)
  {
    uint32_t tok_id = ngram_key.GetWordID_New(order);
    int offset = search_for_id(order, tok_id, start_offset, end_offset);

    if (offset == -1)
      break;

    get_lm_tuple(order, offset, lm_tup);

    ret_order++;
    //ids.push_back(tok_id);

    start_offset = lm_tup.nlevel_offset;
    end_offset = lm_tup.nlevel_offset + lm_tup.nlevel_entries - 1;
  }

  if (ret_order == 0)
  {
    ngram_ret.order = 0;
    ngram_ret.prob = not_in_lm_cost;
    ngram_ret.backoff = 0;
  }
  else
  {
    ngram_ret.order = ret_order;
    memcpy(ngram_ret.word_ids, ngram_key.word_ids, ngram_ret.order * sizeof(uint32_t));
    ngram_ret.prob = lm_tup.prob;
    ngram_ret.backoff = lm_tup.bow;
  }

}


uint32_t ZipLM::MaxUnigramID()
{
  return probs[0]->GetSize() - 1;
}


const uint32_t ZipLM::bits_per_unigram_prob = 8;
const uint32_t ZipLM::bits_per_unigram_bow = 8;
const uint32_t ZipLM::bits_per_bigram_prob = 8;
const uint32_t ZipLM::bits_per_bigram_bow = 8;
const uint32_t ZipLM::bits_per_higher_order_prob = 8;
const uint32_t ZipLM::bits_per_higher_order_bow = 8;


}
