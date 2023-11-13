#include "utils.hpp"

void utils::exit_with_error(const std::string message) {
  log("FATAL: " + message);
  exit(1);
}

void utils::log(const std::string message) {
  std::cout << "[WINGS] " << message << std::endl;
}
