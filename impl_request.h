#pragma once
#include <sstream>

#include "iceweb.h"
namespace sureice {
namespace impl {
class impl_request final : public sureice::request {
 public:
  impl_request() noexcept;
  int parse(const std::string &line) override;

 private:
  int parseURIAndProto(Method m, const std::string &inp);
  int parseBegin(const std::string &inp);
  int parseHeader(const std::string &inp);
  int parseBody(const std::string &inp);

 private:
  std::function<int(const std::string &)> parser;
  int exceptLength;
};
}  // namespace impl
}  // namespace sureice
