// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef KOREKTOR_SERVICE_H
#define KOREKTOR_SERVICE_H

#include "StdAfx.h"

#include "JsonBuilder.h"
#include "RestService.h"

namespace ngramchecker {

struct SpellcheckerDescription {
  string id, file;

  SpellcheckerDescription(string id, string file) : id(id), file(file) {}
};

class KorektorService : public RestService {
 public:
  void init(const vector<SpellcheckerDescription>& spellchecker_descriptions);

  virtual bool handle(RestRequest& req) override;

 private:
  bool handle_correct(RestRequest& req);
  bool handle_suggestions(RestRequest& req);

  class Spellchecker {
   public:
    virtual void suggestions(const string& str, unsigned num, vector<pair<string, vector<string>>>& suggestions) = 0;
  };
  class SpellcheckerProvider {
   public:
    virtual Spellchecker* new_spellchecker() const = 0;
  };
  class KorektorProvider;
  class StripDiacriticsProvider;

  const char* get_data(RestRequest& req, JsonBuilder& error);
  const SpellcheckerProvider* get_provider(RestRequest& req, string& model, JsonBuilder& error);
  unsigned get_suggestions(RestRequest& req, JsonBuilder& error);
  static bool get_line(const char*& data, StringPiece& line);

  unordered_map<string, unique_ptr<SpellcheckerProvider>> spellcheckers;
  string default_model;
  JsonBuilder json_models;
};

} // namespace ngramchecker

#endif
