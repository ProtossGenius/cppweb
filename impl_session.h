#pragma once
#include <boost/asio.hpp>
#include <memory>

#include "iceweb.h"
namespace sureice {
namespace impl {

// impl_session a conn;
class impl_session final : public session {
  typedef boost::asio::ip::tcp tcp;

 public:
  impl_session(web *w, tcp::socket &&_sock)
      : _web(w), _sock(std::move(_sock)) {}

  ~impl_session() { _sock.close(); }

 public:
  void onRequest(std::shared_ptr<request> req) override;
  void sendResponse(std::shared_ptr<response> resp) override;
  void start() override;
  void close() override { _sock.close(); }

 private:
  void delay_close();
  void handle_read_line(std::shared_ptr<request> req,
                        std::shared_ptr<boost::asio::streambuf> buffer);
  void handle_read_size(std::shared_ptr<request> req,
                        std::shared_ptr<boost::asio::streambuf> buffer,
                        size_t size);
  void parse(std::shared_ptr<request> req,
             std::shared_ptr<boost::asio::streambuf> buffer,
             const std::string &s);

 private:
  web *_web;
  tcp::socket _sock;
  long last_run;
};
}  // namespace impl
}  // namespace sureice
