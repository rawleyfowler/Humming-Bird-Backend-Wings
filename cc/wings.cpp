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
void set_callback(http::callback_t);
void set_size_callback(http::size_callback_t);
void run();

static int PORT;
static const char *ADDR;
static http::callback_t CALLBACK;
static http::size_callback_t SIZE_CALLBACK;

extern "C" {
  // NOTE: callback has to be a function pointer, or Rakudo will blow up. Instead of std::function<...>.
void WingsListen(int port, const char *addr,
                 const char *(*callback)(const char *, int, int),
                 unsigned long long (*size_callback)(pid_t)) {
  set_port(port);
  set_addr(addr);
  set_callback(callback);
  set_size_callback(size_callback);
  run();
}
}

void run() {
  http::server server(PORT, std::string(ADDR), CALLBACK, SIZE_CALLBACK);
  server.listen();
  utils::log("FORK OUTTA HERE!");
}

void set_port(int port) { PORT = port; }
void set_addr(const char *addr) { ADDR = addr; }
void set_size_callback(http::size_callback_t size_callback) {
  SIZE_CALLBACK = size_callback;
}
void set_callback(http::callback_t callback) {
  CALLBACK = callback;
}
