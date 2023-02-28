#include "iceweb.h"

#include <fstream>
#include <sstream>

#include "icelog.h"
#include "run_results.h"
#include "str_utils.h"
namespace sureice {
router &router::addRouter(const router &r) {
  sons.emplace_back(r);
  return *this;
}

bool router::range(const std::string &baseURI,
                   std::function<bool(const final_router &)> iter) const {
  std::string currentURI =
      baseURI.rfind("/") == 0 ? baseURI + uri : baseURI + "/" + uri;
  debug(currentURI);
  if (handler != nullptr) {
    iter({currentURI, method, handler});
    if (!sons.empty()) {
      warn(
          "becase the router has handler, ignore his sons, it's URI is "
          ": " +
          currentURI);
    }
    return true;
  }

  if (sons.empty()) {
    warn("the router has neither sons or  handler, it's URI is : " +
         currentURI);
    return false;
  }

  for (const router &r : sons) {
    if (r.range(currentURI, iter)) {
      break;
    }
  }

  return true;
}

void web::addRouter(const router &r) {
  std::lock_guard<std::mutex> _(this->routerMapLocker);
  info("web::addRouter " + r.getURI());
  this->routerMap.insert({r.getURI(), r});
}
void clean2Dot(std::string &filePath) {
  size_t lastSize = filePath.size();
  do {
    lastSize = filePath.size();
    replaceOne(filePath, "/..", "");
    replaceOne(filePath, "../", "");
    replaceOne(filePath, "//", "/");
  } while (lastSize > filePath.size());
}
// addToMap returns: has repeat_key.
bool addToMap(std::map<std::string, Handler> &m, const std::string &key,
              Handler val) {
  std::string uri(key);
  clean2Dot(uri);
  debug("add uri ---> |" + uri + "|");
  if (uri.size() == 0) return true;
  if (uri[0] != '/') uri = "/" + uri;
  if (m.find(uri) != m.end()) {
    return true;
  }
  m.insert({uri, val});
  return false;
}

run_result web::parseRouter(
    std::function<void(std::map<std::string, Handler> &,
                       std::map<std::string, Handler> &)>
        action) {
  static const run_result parse_router_fail({-1, "repeat_uri"});
  std::map<std::string, Handler> l_get, l_post;

  {
    std::lock_guard<std::mutex> _(this->routerMapLocker);
    bool exist = false;
    bool has_repeat_uri = false;
    std::string fail_uri;
    for (auto iter = this->routerMap.begin(); iter != routerMap.end(); iter++) {
      exist = iter->second.range([&has_repeat_uri, &fail_uri, &l_get,
                                  &l_post](const final_router &fr) {
        if (has_repeat_uri) {
          return true;  // break;
        }
        info("parse uri" + fr.URI);
        if (fr.method == Method::GET || fr.method == Method::GET_POST) {
          has_repeat_uri =
              has_repeat_uri || addToMap(l_get, fr.URI, fr.handler);
        }

        if (fr.method == Method::POST || fr.method == Method::GET_POST) {
          has_repeat_uri =
              has_repeat_uri || addToMap(l_post, fr.URI, fr.handler);
        }

        if (has_repeat_uri) {
          fail_uri = fr.URI;
        }
        return false;
      }) || exist;
      if (has_repeat_uri) {
        return {RunResults::REPEAT_URI, fail_uri};
      }
    }

    if (!exist) {
      warn("no uri exist!");
    }
  }

  action(l_get, l_post);
  return {RunResults::SUCCESS};
}

void web::resetRouter(std::map<std::string, Handler> _get,
                      std::map<std::string, Handler> _post) {
  {
    std::lock_guard<std::mutex> _(this->getRouterLocker);
    this->getRouter.swap(_get);
  }

  {
    std::lock_guard<std::mutex> _(this->postRouterLocker);
    this->postRouter.swap(_post);
  }
}

void web::updateRouter(std::map<std::string, Handler> _get,
                       std::map<std::string, Handler> _post) {
  for (auto it = _get.begin(); it != _get.end(); it++) {
    std::lock_guard<std::mutex> _(this->getRouterLocker);
    this->getRouter[it->first] = it->second;
  }

  for (auto it = _post.begin(); it != _post.end(); it++) {
    std::lock_guard<std::mutex> _(this->postRouterLocker);
    this->postRouter[it->first] = it->second;
  }
}

bool file_exist(const std::string &p) {
  std::ifstream i(p);
  return i ? true : false;
}

void hnd_file(const request &r, web &w,
              std::function<void(std::shared_ptr<response>)> resp) {
  static const auto indexs = {"./index.html", "./index.htm", "./index.aspx",
                              "./index.asp",  "./index",     "./index."};
  debug("hnd_file execing, uri = " + r.getURI());
  std::string filePath;
  if (r.getURI() != "/") {
    filePath = "./" + r.getURI();
  } else {
    for (auto index : indexs) {
      if (file_exist(index)) {
        filePath = index;
        break;
      }
    }
  }
  clean2Dot(filePath);
  debug("path  = " + filePath);
  std::ifstream f(filePath, std::ios::in);
  if (!f) {
    w.getSpecialHandler(404)(r, w, resp);
    return;
  }
  std::stringstream ss;
  ss << f.rdbuf();
  auto fileResp = std::make_shared<response>(200, "OK");
  fileResp->addHeader("Content-Type:", "text/html; charset=UTF-8");
  fileResp->addHeader("Referrer-Policy", "no-referrer");
  fileResp->setBody(ss.str());
  resp(fileResp);
}

Handler findHandler(const std::string &uri,
                    const std::map<std::string, Handler> &m, std::mutex &locker,
                    Handler dft = nullptr) {
  std::lock_guard<std::mutex> _(locker);
  auto handler = m.find(uri);
  if (handler != m.end()) {
    return handler->second;
  }

  return dft;
}

Handler web::getHandler(const std::string &uri, Method method) const {
  info("get handler .. |" + uri + "|");
  if (method == Method::POST) {
    return findHandler(uri, this->postRouter, this->postRouterLocker, hnd_file);
  }

  if (method == Method::GET) {
    debug("in get");
    return findHandler(uri, this->getRouter, this->getRouterLocker, hnd_file);
  }

  debug("here.");
  auto p = findHandler(uri, this->postRouter, this->postRouterLocker);
  if (p != nullptr) {
    return p;
  }

  return findHandler(uri, this->getRouter, this->getRouterLocker, hnd_file);
}

Handler web::getSpecialHandler(int code) const {
  auto handf = specialHandler.find(code);
  if (handf != specialHandler.end()) {
    return handf->second;
  }

  return specialHandler.find(404)->second;
}

void response::rangeHeader(
    std::function<void(const std::string &key, const std::string &value)>
        ranger) const {
  for (auto it = header.begin(); it != header.end(); ++it) {
    ranger(it->first, it->second);
  }
}

std::string response::to_string() const {
  static const std::string nl = "\r\n";
  std::stringstream ss;
  ss << "HTTP/1.1 " << getCode() << " " << getCodeDesc() << nl;
  this->rangeHeader([&ss](auto key, auto value) {
    if (key == "Content-Length") return;
    ss << key << ": " << value << nl;
  });
  ss << "Content-Length: " << this->getBody().size() << nl;
  ss << nl;
  ss << getBody() << nl;
  return ss.str();
}

class r404 final : public response {
 public:
  r404() : response() {
    static const std::string page(
        "该页面并不存在，请联系懒得一批的管理员换"
        "掉这个丑的一批的404页面。");
    setCode(404, "Can't found page.");
    setBody(page);
    addHeader("Content-Type", "text/html; charset=UTF-8");
    addHeader("Referrer-Policy", "no-referrer");
  }
};

void h404(const request &r, web &,
          std::function<void(std::shared_ptr<response>)> resp) {
  static std::shared_ptr<response> r404o = std::make_shared<r404>();
  resp(r404o);
}

web::web() { setSpecialHandler(404, h404); }

run_result web::updateRouter() {
  return parseRouter([this](std::map<std::string, Handler> &_get,
                            std::map<std::string, Handler> &_post) {
    this->updateRouter(_get, _post);
  });
}

run_result web::resetRouter() {
  return parseRouter([this](std::map<std::string, Handler> &_get,
                            std::map<std::string, Handler> &_post) {
    this->updateRouter(_get, _post);
  });
}
}  // namespace sureice
