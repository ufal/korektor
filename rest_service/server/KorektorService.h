// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef KOREKTOR_SERVICE_H
#define KOREKTOR_SERVICE_H

#include "StdAfx.h"
#include "Configuration.hpp"

#include "JsonBuilder.h"
#include "RestService.h"

namespace ngramchecker {

struct SpellcheckerDescription {
  string id, file;

  SpellcheckerDescription(string id, string file) : id(id), file(file) {}
};

class KorektorService : public RestService {
 public:
  bool init(const vector<SpellcheckerDescription>& spellchecker_descriptions);

  virtual bool handle(RestRequest& req) override;

 private:
  bool handle_strip(RestRequest& req);

  const char* get_data(RestRequest& req, JsonBuilder& error);
  static bool get_line(const char*& data, StringPiece& line);

  unordered_map<string, unique_ptr<Configuration>> spellcheckers;
};

} // namespace ngramchecker

#endif
