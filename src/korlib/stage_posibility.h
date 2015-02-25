// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#ifndef STAGEPOSIBILITY_HPP_
#define STAGEPOSIBILITY_HPP_

#include "common.h"
#include "factor_list.h"

namespace ngramchecker {

class Configuration;
SP_DEF(Configuration);



//TODO: reorganize this class for better cache usage - it's use in the main computation loop, so it shouldn't be that wasteful - the actual literal strings should be
//moved somewhere else
class StagePosibility {
 public:
  StagePosibility_type type;

  virtual float EmmisionProbability() = 0;
  virtual string ToString() = 0;
  virtual bool IsOriginal() = 0;
  virtual size_t UniqueIdentifier() = 0;
  virtual uint32_t FormIdentifier() = 0;
  virtual bool IsUnknown() = 0;
  virtual u16string &To_u16string() = 0;

  StagePosibility(StagePosibility_type _type): type(_type) {}
};

SP_DEF(StagePosibility);


struct StagePosibility_Identifying_comparer : less<StagePosibilityP> {
  bool operator()(const StagePosibilityP& sp1, const StagePosibilityP& sp2)
  {
    return sp1->UniqueIdentifier() < sp2->UniqueIdentifier();
  }
};

struct StagePosibility_Form_comparer : less<StagePosibilityP> {
  bool operator()(const StagePosibilityP& sp1, const StagePosibilityP& sp2)
  {
    return sp1->FormIdentifier() < sp2->FormIdentifier();
  }
};


struct StagePosibility_sort_cost : less<StagePosibilityP> {
  bool operator()(const StagePosibilityP& sp1, const StagePosibilityP& sp2)
  {
    return sp1->EmmisionProbability() < sp2->EmmisionProbability();
  }
};

class StagePosibilityNew : public StagePosibility {
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

  StagePosibilityNew(const FactorList &_factorList, bool _original, const u16string &_word, Configuration *_conf, float error_model_cost);

};

SP_DEF(StagePosibilityNew);

#if 0
class StagePosibility_Letter : public StagePosibility
{

 private:
  UChar letter;

 public:
  StagePosibility_Letter(UChar _letter);

  virtual double EmmisionProbability();

  virtual string ToString();

  virtual bool IsOriginal();

  virtual uint32_t UniqueIdentifier();

  virtual uint32_t FormIdentifier();

  virtual bool IsUnknown();

  uint32_t LetterID();

  static StagePosibilityP SentenceStartSP();

  static StagePosibilityP SentenceEndSP();

};

SP_DEF(StagePosibility_Letter);
#endif

}


#endif /* STAGEPOSIBILITY_HPP_ */
