// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef REST_REQUEST_H
#define REST_REQUEST_H

#include "StdAfx.h"

#include "ResponseGenerator.h"
#include "StringPiece.h"

namespace ngramchecker {

class RestRequest {
 public:
  virtual ~RestRequest() {}

  virtual bool respond_json(StringPiece json) = 0;
  virtual bool respond_json(ResponseGenerator* generator) = 0;
  virtual bool respond_not_found() = 0;

  string url;
  unordered_map<string, string> params;

 protected:
  RestRequest(const string& url) : url(url) {}
};

} // namespace ngramchecker

#endif
