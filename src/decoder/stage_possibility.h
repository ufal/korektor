// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"
#include "morphology/factor_list.h"

namespace ufal {
namespace korektor {

class Configuration;
SP_DEF(Configuration);

//TODO: reorganize this class for better cache usage - it's use in the main computation loop, so it shouldn't be that wasteful - the actual literal strings should be
//moved somewhere else
class StagePossibility {
 public:
  virtual float EmmisionProbability() = 0;
  virtual string ToString() = 0;
  virtual bool IsOriginal() = 0;
  virtual size_t UniqueIdentifier() = 0;
  virtual uint32_t FormIdentifier() = 0;
  virtual bool IsUnknown() = 0;
  virtual u16string &To_u16string() = 0;
};

SP_DEF(StagePossibility);
typedef shared_ptr<vector<vector<StagePossibilityP>>> StagePossibilitiesType;


struct StagePossibility_Form_comparer : less<StagePossibilityP> {
  bool operator()(const StagePossibilityP& sp1, const StagePossibilityP& sp2)
  {
    return sp1->FormIdentifier() < sp2->FormIdentifier();
  }
};


struct StagePossibility_sort_cost : less<StagePossibilityP> {
  bool operator()(const StagePossibilityP& sp1, const StagePossibilityP& sp2)
  {
    float sp1_emission = sp1->EmmisionProbability();
    float sp2_emission = sp2->EmmisionProbability();
    if (sp1_emission < sp2_emission) return true;
    if (sp1_emission > sp2_emission) return false;
    return sp1->UniqueIdentifier() < sp2->UniqueIdentifier();
  }
};

class StagePossibilityNew : public StagePossibility {
  u16string word;
  bool original;
  float emission_prob;
  FactorList factorList;
  uint32_t form_id;
  size_t uniq_id;

 public:
  virtual float EmmisionProbability() { return emission_prob; }
  virtual string ToString();
  virtual size_t UniqueIdentifier() { return uniq_id; }
  virtual uint32_t FormIdentifier() { return form_id; }
  virtual bool IsOriginal() { return original; }
  virtual bool IsUnknown();
  inline FactorList GetFactorList() { return factorList; }
  virtual u16string &To_u16string() { return word; }

  StagePossibilityNew(const FactorList &_factorList, bool _original, const u16string &_word, Configuration *_conf, float error_model_cost);

};

SP_DEF(StagePossibilityNew);

} // namespace korektor
} // namespace ufal
