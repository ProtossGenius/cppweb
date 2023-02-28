#include "icewebimpl.h"

#include "impl_request.h"
#include "impl_web.h"
namespace sureice {
std::shared_ptr<web> new_web() { return std::make_shared<impl::impl_web>(); }

std::shared_ptr<request> new_request() {
  return std::make_shared<impl::impl_request>();
}

std::shared_ptr<response> error_reponse(int code, std::string reason) {
  auto res = std::make_shared<response>();
  res->setCode(code, "FAIL");
  res->setBody(reason);
  res->addHeader("Content-Type:", "text/html; charset=UTF-8");
  res->addHeader("Referrer-Policy", "no-referrer");
  return res;
}
}  // namespace sureice
