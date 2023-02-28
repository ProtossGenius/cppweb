#pragma once
#include <algorithm>
#include <string>
#include <vector>
namespace sureice {
std::string trim(const std::string &str);
std::vector<std::string> split_n(const std::string &str, const std::string &seq,
                                 int n);
inline std::vector<std::string> split_2(const std::string &str,
                                        const std::string &seq) {
  return split_n(str, seq, 2);
}
inline std::vector<std::string> split(const std::string &str,
                                      const std::string &seq) {
  return split_n(str, seq, -1);
}
void replaceOne(std::string &str, const std::string &old,
                const std::string &neew);
void replaceAll(std::string &str, const std::string &old,
                const std::string &neew);
}  // namespace sureice
