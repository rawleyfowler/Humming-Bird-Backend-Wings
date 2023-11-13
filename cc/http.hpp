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

typedef std::function<const char *(const char *, int, int)> callback_t;
typedef std::function<unsigned long long (pid_t)> size_callback_t;

class connection {
  std::shared_ptr<std::chrono::system_clock::time_point> last_active;

public:
  connection();
  ~connection();
};

class server {
  int port;
  std::string addr;
  size_callback_t size_callback;
  callback_t callback;
  int socket_fd;
  struct sockaddr_in socket_addr;
  std::shared_ptr<std::vector<pid_t>> children = std::make_shared<std::vector<pid_t>>();
  bool shutting_down = false;

public:
  server(const int, std::string, callback_t, size_callback_t);
  ~server();

  virtual void kill_children();
  virtual void listen();
  virtual bool is_shutting_down();
  virtual void toggle_shutdown();
};

} // namespace http
