#include "http.hpp"
#include "utils.hpp"

http::connection::connection() {
  this->last_active = std::make_shared<std::chrono::system_clock::time_point>();
}

http::connection::~connection() {}

http::server::server(const int port, std::string addr, callback_t callback) {
  this->port = port;
  this->addr = addr;
  this->callback = callback;

  this->socket_addr.sin_family = AF_INET;
  this->socket_addr.sin_port = htons(this->port);
  this->socket_addr.sin_addr.s_addr = inet_addr(this->addr.c_str());

  int ret;

  if ((this->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    utils::exit_with_error("Encountered an error creating the socket: " +
                           std::to_string(this->socket_fd));
  }

  if ((ret = bind(this->socket_fd, (sockaddr *)&this->socket_addr,
                  sizeof(this->socket_addr))) < 0) {
    utils::exit_with_error("Encountered an error binding to the socket");
  }
}

http::server::~server(){};

void http::server::listen() {
  int ret;
  if ((ret = ::listen(this->socket_fd, 20)) < 0) {
    utils::exit_with_error("Encountered an error trying to listen to socket: " +
                           std::to_string(ret));
  }

  std::ostringstream stream;

  while (true) {
    socklen_t socket_addr_len = sizeof(this->socket_addr);
    int recv_sock =
        accept(socket_fd, (sockaddr *)&this->socket_addr, &socket_addr_len);
    if (recv_sock < 0) {
      utils::log("Failed to accept connection from addr: " +
                 std::string(inet_ntoa(socket_addr.sin_addr)));
    }

    pid_t pid = fork();
    char buffer[BUFFER_SIZE] = {0};

    if (pid == -1) {
      utils::log("Encountered an error trying to fork.");
      write(recv_sock, nullptr, 0); // This is how you tell the client that we
                                    // are closing the connection.
      close(recv_sock);
    } else if (pid == 0) { // In child process
      read(recv_sock, buffer, BUFFER_SIZE);

      const char *response;
      char count = 0;
      while (count++ < 10 &&
             (response = this->callback(buffer, pid)) == nullptr) {
        read(recv_sock, buffer, BUFFER_SIZE);
      }

      if (count == 10) {
        utils::log("Payload size too large for request. Closing connection.");
        write(recv_sock, nullptr, 0);
      } else {
        std::string response_str(response);
        write(recv_sock, response_str.c_str(), response_str.length() - 1);
      }

      close(recv_sock);
      break;
    }
  }
}
