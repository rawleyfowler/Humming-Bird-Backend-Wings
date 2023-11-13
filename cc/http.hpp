#pragma once

#include "utils.hpp"

#include <arpa/inet.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 1024

namespace http {

typedef std::function<const char *(const char *, int)> callback_t;

class connection {
  std::shared_ptr<std::chrono::system_clock::time_point> last_active;

public:
  connection();
  ~connection();
};

class server {
  int port;
  std::string addr;
  callback_t callback;
  int socket_fd;
  struct sockaddr_in socket_addr;

public:
  server(const int, std::string, callback_t);
  ~server();

  virtual void listen();
};

} // namespace http
