#pragma once
#include <boost/asio.hpp>
#include <queue>

#include "iceweb.h"
namespace sureice {
namespace impl {
class impl_session;
struct delay_pair {
  long ms;
  WebDelayAction action;
};

inline bool operator<(const delay_pair &lhs, const delay_pair &rhs) {
  return lhs.ms < rhs.ms;
}

class impl_web final : public sureice::web {
  typedef boost::asio::ip::tcp tcp;

 public:
  impl_web() {}
  ~impl_web() {}

 private:
  run_result _startup(int port) override;

 public:
  void close() override {}
  void async(std::function<void()> slow) override;
  void delay_action(long ms, WebDelayAction action) override;

 private:
  void manage_session(std::shared_ptr<impl_session> s);
  void start(std::shared_ptr<tcp::acceptor> _acceptor);
  void deal_delay();

 private:
  boost::asio::io_service ioc;
  std::list<std::shared_ptr<impl_session>> l;
  std::priority_queue<delay_pair> _queue;
  std::mutex _queueLock;
};
}  // namespace impl
}  // namespace sureice
