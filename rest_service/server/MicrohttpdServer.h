// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef MICROHTTPD_SERVER_H
#define MICROHTTPD_SERVER_H

#include "StdAfx.h"

#include "RestRequest.h"
#include "RestService.h"
#include "microhttpd.h"

namespace ngramchecker {

class MicrohttpdServer {
 public:
  bool start(RestService* service, unsigned port, unsigned max_connections, unsigned timeout, unsigned max_post_size);

 private:
  static int handle_request(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls);
  static void request_completed(void* cls, struct MHD_Connection* connection, void** con_cls, MHD_RequestTerminationCode toe);

  MHD_Daemon* daemon;
  RestService* service;
  unsigned max_post_size;
};

} // namespace ngramchecker

#endif
