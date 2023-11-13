#include "http.hpp"
#include "utils.hpp"

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void set_port(int port);
void set_addr(const char *addr);
void set_callback(const char *(*callback)(const char *, int));
void run();

static int PORT;
static const char *ADDR;
static const char *(*CALLBACK)(const char *, int);

extern "C" {
void WingsListen(int port, const char *addr,
                 const char *(*callback)(const char *, int)) {
  set_port(port);
  set_addr(addr);
  set_callback(callback);
  run();
}
}

void run() {
  http::server server(PORT, std::string(ADDR), CALLBACK);
  server.listen();
  utils::log("FORK OUTTA HERE!");
}

void set_port(int port) { PORT = port; }
void set_addr(const char *addr) { ADDR = addr; }
void set_callback(const char *(*callback)(const char *, int)) {
  CALLBACK = callback;
}
