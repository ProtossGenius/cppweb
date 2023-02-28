#include "impl_request.h"

#include "icelog.h"
#include "str_utils.h"
namespace sureice {
namespace impl {

int impl_request::parseURIAndProto(Method m, const std::string &str) {
  auto inp = trim(str);
  if (inp.size() == 0) {
    return -1;
  }

  auto url_proto = split_2(inp, " ");
  auto &uri = url_proto[0];
  auto uri_params = split(uri, "?");
  if (uri_params.size() > 2) {
    throw request_fail(400, "too much '?' in request, uri is " + inp);
  }

  setURI(uri_params[0]);
  if (uri_params.size() == 2) {
    auto params = split(uri_params[1], "&");
    for (auto param : params) {
      auto kv = split_2(param, "=");
      if (kv.size() < 2) {
        warn("no value " + param);
        setParams(param, "");
        continue;
      }
      setParams(kv[0], kv[1]);
    }
  }

  auto &protoc = url_proto[1];
  if (protoc.size() == 0) {
    throw request_fail(400, "protocol is empty");
  }

  if (protoc.find("HTTP") == protoc.npos) {
    warn("may not a http request " + protoc);
  }

  setProto(protoc);
  parser = [this](auto inp) { return this->parseHeader(inp); };
  return -1;
}
int impl_request::parseBegin(const std::string &str) {
  std::string inp = trim(str);
  if (inp.size() == 0) {
    return -1;
  }
  if (inp.find("GET") == 0) {
    return parseURIAndProto(Method::GET, inp.substr(3));
  }

  if (inp.find("POST") == 0) {
    return parseURIAndProto(Method::POST, inp.substr(4));
  }

  if (inp.find("GET/POST") == 0) {
    return parseURIAndProto(Method::GET_POST, inp.substr(8));
  }

  throw request_fail(400, "unknow Method, input is " + inp);
}
int impl_request::parseHeader(const std::string &inp) {
  auto str = trim(inp);
  if (str.size() == 0) {
    auto length = getHeader("Content Length");
    if (length.size() == 0) {
      return 0;
    }

    exceptLength = std::stoi(length);
    parser = [this](auto inp) { return this->parseBody(inp); };
    return exceptLength;
  }
  auto kv = split_2(str, ": ");
  setHeader(kv[0], kv[1]);
  return -1;
}
int impl_request::parseBody(const std::string &inp) {
  auto body = getBody();
  setBody(body + inp);
  if (inp.size() < exceptLength) {
    exceptLength -= inp.size();
    return exceptLength;
  }

  return 0;
}

impl_request::impl_request() noexcept
    : parser([this](auto inp) { return this->parseBegin(inp); }) {}

int impl_request::parse(const std::string &line) {
  try {
    return parser(line);
  } catch (const request_fail &e) {
    parseFail(e.getCode(), e.what());
  } catch (const std::exception &e) {
    parseFail(400, e.what());
  }
  return 0;
}

}  // namespace impl
}  // namespace sureice
