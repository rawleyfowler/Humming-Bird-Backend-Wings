#include "http.hpp"
#include "utils.hpp"

#include <algorithm>
#include <asm-generic/socket.h>
#include <signal.h>

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler_wrapper(int signal) {
  shutdown_handler(signal);
  exit(0);
}
}

http::connection::connection() {
  this->last_active = std::make_shared<std::chrono::system_clock::time_point>();
}

http::connection::~connection() {}

http::server::server(const int port, std::string addr, http::callback_t callback, http::size_callback_t size_callback) {
  this->port = port;
  this->addr = addr;
  this->callback = callback;
  this->size_callback = size_callback;

  this->socket_addr.sin_family = AF_INET;
  this->socket_addr.sin_port = htons(this->port);
  this->socket_addr.sin_addr.s_addr = inet_addr(this->addr.c_str());

  int ret;

  if ((ret = (this->socket_fd = socket(AF_INET, SOCK_STREAM, 0))) < 0) {
    utils::exit_with_error("Encountered an error creating the socket: " +
                           std::to_string(ret));
  }

  if ((ret = bind(this->socket_fd, (sockaddr *)&this->socket_addr,
                  sizeof(this->socket_addr))) < 0) {
    utils::exit_with_error("Encountered an error binding to the socket: " + std::to_string(ret));
  }

  const int option = 1;
  if ((ret = setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))) < 0) {
    utils::exit_with_error("Encountered an error setting socket options: " + std::to_string(ret));
  }
}

http::server::~server(){};

void http::server::toggle_shutdown() {
  this->shutting_down = !this->shutting_down;
}

bool http::server::is_shutting_down() {
  return this->shutting_down;
}

void http::server::kill_children() {
  std::for_each(this->children.get()->begin(), this->children.get()->end(), [](pid_t &child) {
    kill(child, SIGKILL);
  });
}

void http::server::listen() {
  // Setup SIGINT handler
  shutdown_handler = [this](int signal) {
    if (this->shutting_down) {
      utils::log("Received second SIGINT, killing process...");
      return exit(1);
    }

    this->toggle_shutdown();
    utils::log("Received SIGINT, shutting down gracefully...");

    this->kill_children();
  };

  signal(SIGINT, signal_handler_wrapper);

  int ret;
  if ((ret = ::listen(this->socket_fd, 20)) < 0) {
    utils::exit_with_error("Encountered an error trying to listen to socket: " +
                           std::to_string(ret));
  }

  while (true) {
    socklen_t socket_addr_len = sizeof(this->socket_addr);
    int recv_sock =
        accept(socket_fd, (sockaddr *)&this->socket_addr, &socket_addr_len);
    utils::log("Accepted connection from: " + std::string(inet_ntoa(socket_addr.sin_addr)));
    if (recv_sock < 0) {
      utils::log("Failed to accept connection from addr: " +
                 std::string(inet_ntoa(socket_addr.sin_addr)));
    }

    pid_t pid = fork();

    if (pid == -1) {
      utils::log("Encountered an error trying to fork.");
      write(recv_sock, nullptr, 0); // This is how you tell the client that we
                                    // are closing the connection.
      close(recv_sock);
    } else if (pid == 0) { // In child process
      char buffer[BUFFER_SIZE] = {0};
      int bytes = read(recv_sock, buffer, BUFFER_SIZE);
      const char *response = this->callback(buffer, bytes, getpid());
      size_t response_size = this->size_callback(getpid());
      write(recv_sock, response, response_size);
      break;
    } else {
      this->children.get()->push_back(pid);
    }
  }
}
