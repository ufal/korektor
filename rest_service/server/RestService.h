// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef REST_SERVICE_H
#define REST_SERVICE_H

#include "StdAfx.h"

#include "RestRequest.h"

namespace ngramchecker {

class RestService {
 public:
  virtual bool handle(RestRequest& req) = 0;
};

} // namespace ngramchecker

#endif
