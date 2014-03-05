// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#include <pthread.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>

#include "StdAfx.h"

#include "KorektorService.h"
#include "MicrohttpdServer.h"

static ssize_t syslog_writer(void* /*cookie*/, char const *data, size_t length) {
  syslog(LOG_ERR, "%.*s", int(length), data);
  return length;
}

static cookie_io_functions_t syslog_functions = { nullptr, &syslog_writer, nullptr, nullptr };

static void redirect_stderr_to_syslog() {
  openlog("korektor_server", 0, LOG_USER);
  FILE* syslog = fopencookie(nullptr, "w", syslog_functions);
  if (!syslog) fprintf(stderr, "Cannot open syslog!\n"), exit(1);
  setvbuf(syslog, nullptr, _IOLBF, 0);
  if (stderr) fclose(stderr);
  stderr = syslog;
}

MicrohttpdServer server;
KorektorService service;

int main(int argc, char* argv[]) {
  bool daemonize = false;

  int argi = 1;
  if (argi < argc && strcmp(argv[argi], "-d") == 0) daemonize = true, argi++;

  // Parse arguments
  if (argc - argi < 1 || ((argc - argi) % 2) != 1)
    fprintf(stderr, "Usage: %s [-d] port (spellchecker_name spellchecker_file)*\n", argv[0]), exit(1);

  unsigned port = atoi(argv[argi++]);
  if (!port || port >= (1<<15) + (1<<14))
    fprintf(stderr, "Wrong port number %u!\n", port), exit(1);

  vector<SpellcheckerDescription> spellcheckers;
  for (; argi < argc; argi += 2)
    spellcheckers.emplace_back(argv[argi], argv[argi + 1]);

  // Initialize the service
  if (!service.init(spellcheckers))
    fprintf(stderr, "Cannot load specified models!\n"), exit(1);

  // Daemonize
  if (daemonize) {
    if (daemon(1, 0) != 0)
      fprintf(stderr, "Cannot daemonize %s!\n", argv[0]), exit(1);
    redirect_stderr_to_syslog();
  }

  for (bool first = true; first || daemonize; first = false, first = true) {
    pid_t child = fork();
    if (child == -1) fprintf(stderr, "Failed to fork!\n"), exit(1);

    if (!child) {
      // Starting the server
      if (!server.start(&service, port, 100, 60, 1<<20))
        fprintf(stderr, "Cannot start korektor_server!\n"), exit(42);

      fprintf(stderr, "Successfully started korektor_server.\n");

      // All is fine, wait indefinitely
      while (true) pause();
      exit(0);
    }

    int status;
    if (waitpid(child, &status, 0) != child) fprintf(stderr, "Failed to wait for korektor_server.\n"), exit(1);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 42) exit(1);
  }

  return 0;
}
