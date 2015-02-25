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

namespace ufal {
namespace korektor {

ZipLM::ZipLM(const string &_factor_name, uint32_t _order, double _not_in_lm_cost, vector<vector<double> > &_probs, vector<vector<double> > &_bows,
             vector<vector<uint32_t> > &_ids, vector<vector<uint32_t> > &_offsets)
{
  factor_name = _factor_name;
  lm_order = _order;
  not_in_lm_cost = _not_in_lm_cost;

  max_unigram_id = _probs[0].size() - 1;
  cerr << "LM order: " << _order << endl;

  ids.push_back(MyPackedArrayP());

  for (uint32_t i = 0; i < _order; i++)
  {
    uint32_t bits_per_prob;

    if (i == 0)
      bits_per_prob = bits_per_unigram_prob;
    else if (i == 1)
      bits_per_prob = bits_per_bigram_prob;
    else
      bits_per_prob = bits_per_higher_order_prob;

    cerr << "creating probs order " << i << endl;

    probs.push_back(MyMappedDoubleArrayP(new MyMappedDoubleArray(_probs[i], bits_per_prob)));
  }

  for (uint32_t i = 0; i < _order - 1; i++)
  {
    uint32_t bits_per_bow;

    if (i == 0)
      bits_per_bow = bits_per_unigram_bow;
    else if (i == 1)
      bits_per_bow = bits_per_bigram_bow;
    else
      bits_per_bow = bits_per_higher_order_bow;

    cerr << "creating bows order " << i << endl;
    bows.push_back(MyMappedDoubleArrayP(new MyMappedDoubleArray(_bows[i], bits_per_bow)));
  }

  for (uint32_t i = 1; i < _order; i++)
  {
    cerr << "creating ids order " << i << endl;
    ids.push_back(MyPackedArrayP(new MyPackedArray(_ids[i])));
  }

  for (uint32_t i = 0; i < _order - 1; i++)
  {

    offsets.push_back(CompIncreasingArrayP(new CompIncreasingArray(_offsets[i], ids[i + 1]->GetSize() - 1)));
  }
}

ZipLM::ZipLM(string bin_file)
{
  filename = bin_file;
  ifstream ifs;
  ifs.open(bin_file.c_str(), ios::in | ios::binary);

//  if (! ifs.is_open())
//  {
//    cerr << "Opening: " << bin_file << endl;
//  }
  assert(ifs.is_open());

  factor_name = MyUtils::ReadString(ifs);

  ifs.read((char*)&lm_order, sizeof(uint32_t));
  ifs.read((char*)&not_in_lm_cost, sizeof(double));

  ids.push_back(MyPackedArrayP());

  for (uint32_t i = 0; i < lm_order; i++)
  {
    probs.push_back(MyMappedDoubleArrayP(new MyMappedDoubleArray(ifs)));
  }

  max_unigram_id = probs[0]->GetSize() - 1;

  for (uint32_t i = 0; i < lm_order - 1; i++)
  {
    bows.push_back(MyMappedDoubleArrayP(new MyMappedDoubleArray(ifs)));
  }

  for (uint32_t i = 1; i < lm_order; i++)
  {
    ids.push_back(MyPackedArrayP(new MyPackedArray(ifs)));
  }

  for (uint32_t i = 0; i < lm_order - 1; i++)
  {
    offsets.push_back(CompIncreasingArrayP(new CompIncreasingArray(ifs)));
  }

  ifs.close();

}


void ZipLM::SaveInBinaryForm(string out_file)
{
  cerr << "Saving in binary form..." << endl;
  ofstream ofs;
  ofs.open(out_file.c_str(), ios::out | ios::binary);
  assert(ofs.is_open());
  MyUtils::WriteString(ofs, factor_name);
  ofs.write((char*)&lm_order, sizeof(uint32_t));
  ofs.write((char*)&not_in_lm_cost, sizeof(double));

  for (uint32_t i = 0; i < lm_order; i++)
  {
    probs[i]->WriteToStream(ofs);
  }

  for (uint32_t i = 0; i < lm_order - 1; i++)
  {
    bows[i]->WriteToStream(ofs);
  }

  for (uint32_t i = 1; i < lm_order; i++)
  {
    ids[i]->WriteToStream(ofs);
  }

  for (uint32_t i = 0; i < lm_order - 1; i++)
  {
    offsets[i]->WriteToStream(ofs);
  }


  ofs.close();
}

/// @brief Creates the language model instance from the ARPA format language model stored in 'text_file'
///
/// @param text_file N-gram text file in ARPA format
/// @param morphology Morphology
/// @param _factor_name Factor name
/// @param lm_order LM order
/// @param not_in_lm_cost @todo variable for ?
/// @return Language model object of type @ref ZipLM
ZipLMP ZipLM::createFromTextFile(string text_file, MorphologyP &morphology, string _factor_name, uint lm_order, double not_in_lm_cost)
{
  cerr << "Creating from text file..." << endl;
  ifstream ifs;
  ifs.open(text_file.c_str());
  assert(ifs.is_open());

  string s;
  uint32_t max_id = 0;

  int last_unigram_id = -1;
  int unigram_id;
  uint factorIndex = morphology->GetFactorMap()[_factor_name];

  cerr << "factorIndex = " << factorIndex << endl;

  vector<vector<double> > probs;
  vector<vector<double> > bows;
  vector<vector<uint32_t> > ids;
  vector<vector<uint32_t> > offsets;

  ids.push_back(vector<uint32_t>());

  for (uint32_t i = 0; i < lm_order; i++)
  {
    probs.push_back(vector<double>());

    //if (i > 0)
    ids.push_back(vector<uint32_t>());

    if (i < lm_order - 1)
    {
      offsets.push_back(vector<uint32_t>());
      bows.push_back(vector<double>());
    }
  }

  set<NGram, NGram_compare> ngrams;

  uint ngram_order = 0;
  vector<string> toks;

  while (MyUtils::SafeReadline(ifs, s))
  {
    if (s.empty()) continue;
    else if (s.find("ngram") == 0) continue;
    else if (s.find("-grams:") != string::npos)
    {
      ngram_order = s[1] - '0';
      cerr << "reading " << ngram_order << "grams...\n";
    }
    else if (s[0] == '\\') continue;
    else
    {
      assert(ngram_order > 0);
      MyUtils::Split(toks, s, " \t");
      FATAL_CONDITION(toks.size() == ngram_order + 1 || toks.size() == ngram_order + 2, "corrupted line: " << s);

      double bow = 0;
      double prob = -MyUtils::my_atof(toks[0]);

      if (prob > 90) prob = not_in_lm_cost;

      uint32_t *ids = new uint32_t[ngram_order];

      bool all_known = true;
      for (uint i = 0; i < ngram_order; i++)
      {
        int _id = morphology->GetFactorID(factorIndex, toks[i + 1]);
        if (_id == -1)
          all_known = false;
        else
        {
          //!!! Order of storing IDs is changed to the original way (like in the master thesis submission) - i.e. reversed order (next word, history new, history old...)
          //It allows more convenient look-up
          ids[ngram_order - i - 1] = (uint)_id;
        }

        if (ngram_order == 1 && all_known == true && _id > (int)max_id)
          max_id = _id;
      }

      if (all_known == false)
      {
        delete[] ids;
        continue;
      }

      if (toks.size() == ngram_order + 2)
        bow = - MyUtils::my_atof(toks.back());

      if (bow > 90) bow = not_in_lm_cost;

      ngrams.insert(NGram(ngram_order, ids, prob, bow));
    }
  }

  set<NGram, NGram_compare> added_ngrams;

  cerr << "adding aux nodex..." << endl;

  uint num_added_ngrams = 0;

  for (auto it = ngrams.begin(); it!= ngrams.end(); it++)
  {
    NGram aux = *it;
    if (aux.order > 1)
    {
      aux.order--;
      while (aux.order > 0 && ngrams.find(aux) == ngrams.end() && added_ngrams.find(aux) == added_ngrams.end())
      {
        NGram aux2 = aux;
        aux2.order--;

        while (ngrams.find(aux2) == ngrams.end())
        {
          FATAL_CONDITION(aux2.order > 0, "");
          aux2.order--;
        }

        aux.prob = ngrams.find(aux2)->prob;
        aux.backoff = 0;

        cerr << "adding aux node:";

        for (int i = aux.order - 1; i >= 0; i--)
          cerr << " " << morphology->GetFactorString(factorIndex, aux.word_ids[i]);
        cerr << " ~ " << aux.prob << endl;

        added_ngrams.insert(aux);
        aux.order--;
        num_added_ngrams++;
      }
    }
  }

  cerr << "!!!!!!!!! num_added_ngrams = " << num_added_ngrams << endl;

  for (auto it = added_ngrams.begin(); it != added_ngrams.end(); it++)
  {
    ngrams.insert(*it);
  }

  cerr << "inserting missing unigrams... (maxID = " << max_id << ")" << endl;

  NGram aux_unigram = NGram(1);
  aux_unigram.prob = not_in_lm_cost;
  aux_unigram.backoff = 0.0;

  for (uint i = 0; i < max_id; i++)
  {
    aux_unigram.word_ids[0] = i;

    if (ngrams.find(aux_unigram) == ngrams.end())
      ngrams.insert(aux_unigram);
  }

  cerr << "creating ZipTBO structures..." << endl;

  uint32_t counter = 0;
  for (auto it = ngrams.begin(); it != ngrams.end(); it++)
  {
    counter++;
    if (counter % 10000 == 0) cerr << counter << endl;
    NGram ngram = *it;
    FATAL_CONDITION(ngram.order > 0, "");
    uint array_index = ngram.order - 1;
    if (ngram.order < lm_order)
    {
      offsets[array_index].push_back(ids[array_index + 1].size());
      bows[array_index].push_back(ngram.backoff);
    }

    probs[array_index].push_back(ngram.prob);

    uint32_t new_id = ngram.word_ids[ngram.order - 1];

    if (ngram.order > 1)
    {
      ids[array_index].push_back(new_id);
    }
    else
    {
      unigram_id = new_id;

      FATAL_CONDITION(unigram_id == last_unigram_id + 1, unigram_id << " != " << last_unigram_id);
      last_unigram_id = unigram_id;
    }

  }

  cerr << "binarization..." << endl;

  ZipLMP ret_lm = ZipLMP(new ZipLM(_factor_name, lm_order, not_in_lm_cost, probs, bows, ids, offsets));


  cerr << "testing..." << endl;
  uint test_counter = 0;
  for (auto it = ngrams.begin(); it != ngrams.end(); it++)
  {
    if (test_counter % 50000 == 0) cerr << "testing: " << test_counter << endl;
    test_counter++;

    //if (test_counter > 3800000) cerr << "finish: " << test_counter << endl;

    NGram key = *it;
    NGram val = NGram(key.order);
    ret_lm->GetNGramForNGramKey(key, val);

    FATAL_CONDITION(val.order == key.order, "");
    FATAL_CONDITION(fabs(val.prob - key.prob) < 0.5, fabs(val.prob - key.prob));
    FATAL_CONDITION(fabs(val.backoff - key.backoff) < 0.5, fabs(val.prob - key.prob));
  }

  cerr << "OK!" << endl;
  return ret_lm;
}

} // namespace korektor
} // namespace ufal
