#pragma once
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
namespace sureice {

inline long now() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
enum Method {
  POST,
  GET,
  GET_POST,  // both support
};

class response;

// request http request.
class request {
 public:
  virtual ~request() {}
  // parse return:{
  // -1  : read line
  // 0   : finish read
  // > 0 : how many bytes to read
  // }
  virtual int parse(const std::string &line) = 0;
  Method getMethod() { return method; }
  const std::string &getURI() const { return URI; }
  const std::string &getProto() const { return proto; }
  std::string getParams(const std::string &key) const {
    auto it = params.find(key);
    return it == params.end() ? "" : it->second;
  }
  std::string getHeader(const std::string &key) const {
    auto it = header.find(key);
    return it == header.end() ? "" : it->second;
  }
  const std::string &getBody() const { return body; }
  bool success() { return code == 0; }
  void parseFail(int code, const std::string errorInfo) {
    this->code = code;
    this->body = errorInfo;
  }

 protected:
  void setMethod(Method method) { this->method = method; }
  void setURI(const std::string &URI) { this->URI = URI; }
  void setParams(const std::string &key, const std::string &value) {
    this->params[key] = value;
  }
  void setHeader(const std::string &key, const std::string &value) {
    this->header[key] = value;
  }
  void setBody(const std::string &body) { this->body = body; }
  void setProto(const std::string &proto) { this->proto = proto; }

 private:
  Method method;
  std::string proto;
  std::string URI;
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> header;
  std::string body;
  int code = 0;
};
class web;
typedef std::function<void(const request &, web &,
                           std::function<void(std::shared_ptr<response>)>)>
    Handler;

struct final_router final {
  std::string URI;
  Method method;
  Handler handler;
};

// router contains path(/...) and handler(what to do).
class router final {
 public:
  router(std::string uri, Method method, Handler handler = nullptr)
      : uri(uri), method(method), handler(handler) {}
  router(std::string uri) : uri(uri) {}

 public:
  ~router() {}
  router &addRouter(const router &r);
  // range returns:if exist. iter::returns if need break.
  bool range(std::function<bool(const final_router &)> iter) const {
    return range("", iter);
  }
  const std::string &getURI() const { return uri; }

 private:
  bool range(const std::string &baseURI,
             std::function<bool(const final_router &)> iter) const;

 private:
  std::string uri;
  Method method;
  Handler handler;
  std::list<router> sons;
};

// response http response.
class response {
 public:
  virtual ~response() {}
  response(int code, const std::string &codeDesc = "")
      : code(code), codeDesc(codeDesc) {}
  response() {}

 public:
  virtual std::string to_string() const;
  void setCode(int code, const std::string &desc = "") {
    this->code = code;
    this->codeDesc = desc;
  }

  void setBody(const std::string &body) { this->body = body; }

  void addHeader(const std::string &key, const std::string &val) {
    header.push_back({key, val});
  }

 protected:
  int getCode() const { return code; }
  const std::string &getBody() const { return this->body; }
  const std::string &getCodeDesc() const { return this->codeDesc; }
  void rangeHeader(
      std::function<void(const std::string &key, const std::string &value)>
          ranger) const;

 private:
  int code;
  std::string codeDesc;
  std::list<std::pair<std::string, std::string>> header;
  std::string body;
};

// run_result web's startup returns.
struct run_result {
  int code;  // internal error's code always less than 0;
  std::string reason;

  bool sucess() { return code == 0; }
};

typedef std::function<void(web &)> WebDelayAction;

// web http web. you need init special Handler, for example 404 page.
class web {
 public:
  web();  // will init specialHandlers here.
  web(std::map<int, Handler> specialHandler) : specialHandler(specialHandler) {}
  virtual ~web() {}
  run_result startup(int port) {
    auto parseResult = resetRouter();
    if (!parseResult.sucess()) {
      return parseResult;
    }

    return _startup(port);
  }
  void addRouter(const router &r);

 public:
  virtual void close() = 0;
  virtual void async(std::function<void()> slow) = 0;
  virtual void delay_action(long ms, WebDelayAction action) = 0;
  web &multiAddRouter(const router &r) {
    addRouter(std::move(r));
    return *this;
  }

  void setSpecialHandler(int code, Handler h) {
    if (h == nullptr) return;
    specialHandler[code] = h;
  }

 public:
  Handler getHandler(const std::string &uri, Method method) const;
  Handler getSpecialHandler(int code) const;
  run_result updateRouter();
  run_result resetRouter();

 private:
  virtual run_result _startup(int port) = 0;
  run_result parseRouter(
      std::function<void(std::map<std::string, Handler> &_get,
                         std::map<std::string, Handler> &_post)>
          action);
  void resetRouter(std::map<std::string, Handler> _get,
                   std::map<std::string, Handler> _post);
  void updateRouter(std::map<std::string, Handler> _get,
                    std::map<std::string, Handler> _post);

 private:
  std::map<int, Handler> specialHandler;
  std::map<std::string, router> routerMap;
  std::map<std::string, Handler> getRouter, postRouter;
  mutable std::mutex routerMapLocker, getRouterLocker, postRouterLocker;
};

// per socket connect.
class session {
 public:
  virtual void onRequest(std::shared_ptr<request>) = 0;
  virtual void sendResponse(std::shared_ptr<response> resp) = 0;
  virtual void start() = 0;
  virtual void close() = 0;
};

class request_fail : public std::runtime_error {
 public:
  request_fail(int code, const std::string &message)
      : runtime_error(message), code(code) {}

 public:
  int getCode() const { return code; }

 private:
  int code;
};

}  // namespace sureice
