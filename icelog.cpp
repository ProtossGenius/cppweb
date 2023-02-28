#include "icelog.h"

#include "iostream"
namespace sureice {
void debug(const std::string &str) {
  std::cout << "[debug]" << str << std::endl;
}
void info(const std::string &str) {
  std::cout << "[info ]" << str << std::endl;
}
void warn(const std::string &str) {
  std::cout << "[warn ]" << str << std::endl;
}
void error(const std::string &str) {
  std::cout << "[error]" << str << std::endl;
}
}  // namespace sureice
