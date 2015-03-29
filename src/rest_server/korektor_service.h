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
  string id, file;

  SpellcheckerDescription(string id, string file) : id(id), file(file) {}
};

class KorektorService : public ufal::microrestd::rest_service {
 public:
  void Init(const vector<SpellcheckerDescription>& spellchecker_descriptions);

  virtual bool handle(ufal::microrestd::rest_request& req) override;

 private:
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
  unordered_map<string, unique_ptr<SpellcheckerProvider>> spellcheckers;

  // REST service
  bool HandleCorrect(ufal::microrestd::rest_request& req);
  bool HandleSuggestions(ufal::microrestd::rest_request& req);

  const string* GetData(ufal::microrestd::rest_request& req, string& error);
  const SpellcheckerProvider* GetProvider(ufal::microrestd::rest_request& req, string& model, string& error);
  const string& GetInputFormat(ufal::microrestd::rest_request& req);
  unsigned GetSuggestions(ufal::microrestd::rest_request& req, string& error);

  string default_model;
  ufal::microrestd::json_builder json_models;
};

} // namespace korektor
} // namespace ufal
