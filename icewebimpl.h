#pragma once
#include "iceweb.h"

namespace sureice {
std::shared_ptr<web> new_web();
std::shared_ptr<request> new_request();
std::shared_ptr<response> error_reponse(int code, std::string reason);
}  // namespace sureice
