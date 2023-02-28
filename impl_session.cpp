#include "impl_session.h"

#include "icelog.h"
#include "iceweb.h"
#include "icewebimpl.h"
//#include "assert.h"
namespace sureice {
namespace impl {

void impl_session::onRequest(std::shared_ptr<request> req) {
  last_run = now();
  auto hand = _web->getHandler(req->getURI(), req->getMethod());
  try {
    hand(*req, *_web,
         [this](std::shared_ptr<response> resp) { this->sendResponse(resp); });
  } catch (const request_fail &e) {
    this->sendResponse(error_reponse(e.getCode(), e.what()));
  } catch (const std::exception &e) {
    this->sendResponse(error_reponse(400, e.what()));
  }
}

void impl_session::delay_close() {
  if (last_run + 9000 > now()) {
    _web->delay_action(now() - last_run,
                       [this](web &) { this->delay_close(); });
    return;
  }

  this->close();
}

void impl_session::start() {
  last_run = now();
  delay_close();
  auto req = new_request();
  std::shared_ptr<boost::asio::streambuf> buffer =
      std::make_shared<boost::asio::streambuf>();
  handle_read_line(req, buffer);
}
void impl_session::parse(std::shared_ptr<request> req,
                         std::shared_ptr<boost::asio::streambuf> buffer,
                         const std::string &s) {
  last_run = now();
  int parse_code = req->parse(s);
  if (parse_code == 0) {
    this->onRequest(req);
    this->start();
    return;
  } else if (parse_code == -1) {
    handle_read_line(req, buffer);
  } else {
    handle_read_size(req, buffer, parse_code);
  }
}
void impl_session::handle_read_line(
    std::shared_ptr<request> req,
    std::shared_ptr<boost::asio::streambuf> buffer) {
  last_run = now();
  boost::asio::async_read_until(
      _sock, *buffer, '\n',
      [req, buffer, this](const boost::system::error_code &err,
                          size_t bytes_transferred) {
        if (err.failed()) {
          if (err != boost::asio::error::eof) {
            error(
                "impl_session::handle_read_line callback fail, "
                "reason is : " +
                err.message());
          }
          return;
        }
        std::string line;
        line.resize(bytes_transferred);
        buffer->sgetn(&line[0], bytes_transferred);
        this->parse(req, buffer, line);
      });
}

void impl_session::handle_read_size(
    std::shared_ptr<request> req,
    std::shared_ptr<boost::asio::streambuf> buffer, size_t size) {
  last_run = now();
  boost::asio::async_read_until(
      this->_sock, *buffer, '\n',
      [req, buffer, this, size](const boost::system::error_code &err,
                                size_t bytes_transferred) {
        if (err.failed()) {
          if (err != boost::asio::error::eof) {
            error(
                "impl_session::handle_read_line callback fail, "
                "reason is : " +
                err.message());
          }
          return;
        }
        if (buffer->size() < size) {
          this->handle_read_size(req, buffer, size);
          return;
        }
        std::string s;
        s.resize(size);
        buffer->sgetn(&s[0], size);
        this->parse(req, buffer, s);
      });
}

void impl_session::sendResponse(std::shared_ptr<response> resp) {
  last_run = now();
  std::string toSend = resp->to_string();
  size_t toSendSize = toSend.size();
  _sock.async_send(
      boost::asio::buffer(toSend),
      [this, toSendSize](const boost::system::error_code &err,
                         size_t bytes_transferred) {
        if (toSendSize != bytes_transferred) {
          error("not eqa.");
        }
        //_assert2(bytes_transferred, toSendSize);
        if (err.failed()) {
          error("sendResponse error, message is : " + err.message());
          return;
        }

        this->start();
      });
}

}  // namespace impl
}  // namespace sureice
