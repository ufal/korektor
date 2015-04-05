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
#include "microrestd/microrestd.h"
#include "spellchecker/spellchecker_correction.h"
#include "token/input_format.h"

namespace ufal {
namespace korektor {

struct SpellcheckerDescription {
  string id;
  string file;
  string acknowledgements;

  SpellcheckerDescription(const string& id, const string& file, const string& acknowledgements) :
    id(id), file(file), acknowledgements(acknowledgements) {}
};

class KorektorService : public ufal::microrestd::rest_service {
 public:
  bool Init(const vector<SpellcheckerDescription>& spellchecker_descriptions);

  virtual bool handle(ufal::microrestd::rest_request& req) override;

 private:
  static unordered_map<string, bool (KorektorService::*)(ufal::microrestd::rest_request&)> handlers;

  // Generic Spellchecker interface
  class SpellcheckerI {
   public:
    virtual void Suggestions(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned alternatives) = 0;
  };
  class SpellcheckerProvider {
   public:
    virtual LexiconP Lexicon() const = 0;
    virtual SpellcheckerI* NewSpellchecker() const = 0;
  };
  class KorektorProvider;
  class StripDiacriticsProvider;

  struct SpellcheckerModel {
    string id;
    string acknowledgements;
    unique_ptr<SpellcheckerProvider> spellchecker;

    SpellcheckerModel(string id, string acknowledgements, SpellcheckerProvider* spellchecker) :
      id(id), acknowledgements(acknowledgements), spellchecker(spellchecker) {}
  };
  vector<SpellcheckerModel> spellcheckers;
  unordered_map<string, const SpellcheckerModel*> spellcheckers_map;
  const SpellcheckerModel* LoadSpellchecker(const string& id, string& error);

  // REST service
  bool HandleModels(ufal::microrestd::rest_request& req);
  bool HandleCorrect(ufal::microrestd::rest_request& req);
  bool HandleSuggestions(ufal::microrestd::rest_request& req);

  static void CommonResponse(const SpellcheckerModel* model, ufal::microrestd::json_builder& json);

  const string* GetData(ufal::microrestd::rest_request& req, string& error);
  const string& GetModelId(ufal::microrestd::rest_request& req);
  const string& GetInputFormat(ufal::microrestd::rest_request& req);
  unsigned GetSuggestions(ufal::microrestd::rest_request& req, string& error);

  ufal::microrestd::json_builder json_models;
};

} // namespace korektor
} // namespace ufal
