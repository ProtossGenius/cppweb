#include "impl_web.h"

#include <ctime>
#include <string>
#include <thread>

#include "icelog.h"
#include "icewebimpl.h"
#include "impl_session.h"
#include "run_results.h"
namespace sureice {
namespace impl {

void impl_web::async(std::function<void()> slow) {
  // TODO: 这里可以用C++2a的携程
  std::thread t(slow);
  t.detach();
}

void impl_web::delay_action(long ms, WebDelayAction action) {
  std::lock_guard<std::mutex> _(this->_queueLock);
  this->_queue.push({now() + ms, action});
}

void impl_web::deal_delay() {
  while (!_queue.empty()) {
    std::lock_guard<std::mutex> _(this->_queueLock);
    auto it = std::move(_queue.top());
    if (it.ms > now()) {
      break;
    }
    ioc.post([this, it]() { it.action(*this); });
    _queue.pop();
  }

  ioc.post([this]() { this->deal_delay(); });
}

run_result impl_web::_startup(int port) {
  this->ioc.post([this]() { this->deal_delay(); });
  tcp::endpoint endpoint(tcp::v4(), port);
  std::shared_ptr<tcp::acceptor> _acceptor =
      std::make_shared<tcp::acceptor>(ioc, endpoint);
  start(_acceptor);
  info("sureiceweb startup on port [" + std::to_string(port) + "]");
  ioc.run();
  return {RunResults::SUCCESS};
}

void impl_web::start(std::shared_ptr<tcp::acceptor> _acceptor) {
  std::shared_ptr<tcp::socket> _sock = std::make_shared<tcp::socket>(ioc);
  _acceptor->async_accept(
      *_sock, [_acceptor, this, _sock](boost::system::error_code ec) {
        if (!ec) {
          std::shared_ptr<impl_session> s =
              std::make_shared<impl_session>(this, std::move(*_sock));
          s->start();
          this->manage_session(s);
        } else {
          error("when accept:" + ec.message());
        }
        start(_acceptor);
      });
}

void impl_web::manage_session(std::shared_ptr<impl_session> s) {
  l.push_back(s);
}

}  // namespace impl
}  // namespace sureice
