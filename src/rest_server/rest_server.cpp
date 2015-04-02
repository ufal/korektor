// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <cstdio>
#include <sstream>

#include "common.h"
#include "korektor_service.h"
#include "microrestd/microrestd.h"
#include "utils/options.h"
#include "utils/parse.h"
#include "version/version.h"

using namespace ufal::korektor;

// On Linux define streambuf writing to syslog
#ifdef __linux__
#include <streambuf>
#include <syslog.h>
#include <unistd.h>

class syslog_streambuf : public streambuf {
 public:
  virtual ~syslog_streambuf() {
    syslog_write();
  }

 protected:
  virtual int overflow(int c) override {
    if (c != EOF && c != '\n')
      buffer.push_back(c);
    else
      syslog_write();
    return c;
  }

 private:
  string buffer;

  void syslog_write() {
    if (!buffer.empty()) {
      syslog(LOG_ERR, "%s", buffer.c_str());
      buffer.clear();
    }
  }
};
#endif

ufal::microrestd::rest_server server;
KorektorService service;

int main(int argc, char* argv[]) {
  iostream::sync_with_stdio(false);

  Options::Map options;
  if (!Options::Parse({{"daemon", Options::Value::none},
                       {"version", Options::Value::none},
                       {"help", Options::Value::none}}, argc, argv, options) ||
      options.count("help") ||
      (!options.count("version") && (argc < 2 || (argc % 3) != 2)))
    runtime_failure("Usage: " << argv[0] << " [options] port (model_name configuration_file acknowledgements)*\n"
                    "Options: --daemon\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version")) {
    ostringstream other_libraries;
    auto microrestd = ufal::microrestd::version::current();
    other_libraries << "MicroRestD " << microrestd.major << '.' << microrestd.minor << '.' << microrestd.patch;
    cout << version::version_and_copyright(other_libraries.str()) << endl;
    return 0;
  }

  // Process options
  int port = Parse::Int(argv[1], "port number");

#ifndef __linux__
  if (options.count("daemon")) runtime_failure("The --daemon option is currently supported on Linux only!");
#endif

  // Initialize the service
  vector<SpellcheckerDescription> spellcheckers;
  for (int i = 2; i < argc; i += 3)
    spellcheckers.emplace_back(argv[i], argv[i + 1], argv[i + 2]);

  // Initialize the service
  if (!service.Init(spellcheckers))
    runtime_failure("Cannot load specified models!");

  // Open log file
  string log_file_name = string(argv[0]) + ".log";
  FILE* log_file = fopen(log_file_name.c_str(), "w");
  if (!log_file) runtime_failure("Cannot open log file '" << log_file_name << "' for writing!");

  // Daemonize if requested
#ifdef __linux__
  if (options.count("daemon")) {
    if (daemon(1, 0) != 0)
      runtime_failure("Cannot daemonize in '" << argv[0] << "' executable!");

    // Redirect cerr to syslog
    openlog("korektor_server", 0, LOG_USER);
    static syslog_streambuf syslog;
    cerr.rdbuf(&syslog);
  }
#endif

  // Start the server
  server.set_log_file(log_file);
  server.set_max_connections(512);
  server.set_max_request_body_size(1 << 19);
  server.set_min_generated(2048);
  server.set_threads(0);
  server.set_timeout(60);

  if (!server.start(&service, port))
    runtime_failure("Cannot start korektor_server'!");

  cerr << "Successfully started korektor_server on port " << port << "." << endl;

  // Wait until finished
  server.wait_until_signalled();
  server.stop();
  fclose(log_file);

  return 0;
}
