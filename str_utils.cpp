#include "str_utils.h"

#include <ctype.h>
namespace sureice {
std::string trim(const std::string &str) {
  if (str.empty()) {
    return str;
  }

  int begin = 0, end = str.size() - 1;
  for (; begin < str.size(); ++begin) {
    if (!isspace(str[begin])) break;
  }

  for (; end >= 0; --end) {
    if (!isspace(str[end])) break;
  }

  ++end;
  return str.substr(begin, end);
}
std::vector<std::string> split_n(const std::string &str, const std::string &seq,
                                 int n) {
  std::vector<std::string> res;
  int ptr = 0;
  size_t nextPos = str.find(seq);
  while (n - 1 != res.size()) {
    if (nextPos == str.npos) {
      res.emplace_back(str.substr(ptr));
      return res;
    }

    res.emplace_back(str.substr(ptr, nextPos));
    ptr = nextPos + seq.size();
    nextPos = str.find(seq, ptr);
  }
  if (ptr != str.size()) {
    res.emplace_back(str.substr(ptr));
  }
  return res;
}

void replaceOne(std::string &str, const std::string &old,
                const std::string &neew) {
  auto pos = str.find(old);
  if (pos == str.npos) {
    return;
  }

  str.replace(pos, old.size(), neew);
}
void replaceAll(std::string &str, const std::string &old,
                const std::string &neew) {
  size_t lastSize = str.size();
  do {
    replaceOne(str, old, neew);
  } while (lastSize > str.size());
}

}  // namespace sureice
