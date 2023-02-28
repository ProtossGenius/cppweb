#include "iceweb/icewebimpl.h"
#include <iostream>
int main(int args, char **argv) {
    auto web  = sureice::new_web();
    int  port = 8080;
    if (args > 1) {
        port = std::stoi(argv[ 1 ]);
    }
    std::list<int> l;
    l.emplace_front();
    web->addRouter({"/sureice", sureice::Method::GET_POST,
                    [](auto &req, auto &w, auto sresp) {
                        std::shared_ptr<sureice::response> resp =
                            std::make_shared<sureice::response>();
                        resp->addHeader("Content-Type",
                                        "text/html; charset=UTF-8");
                        resp->addHeader("Referrer-Policy", "no-referrer");
                        resp->setBody("这是一个自定义的路由");
                        sresp(resp);
                    }});
    sureice::router secRouter("/second");
    secRouter
        .addRouter({"iceweb", sureice::Method::GET_POST,
                    [](auto &req, auto &w, auto sresp) {
                        std::shared_ptr<sureice::response> resp =
                            std::make_shared<sureice::response>();
                        resp->addHeader("Content-Type",
                                        "text/html; charset=UTF-8");
                        resp->addHeader("Referrer-Policy", "no-referrer");
                        resp->setBody("这是一个自定义的多级路由");
                        sresp(resp);
                    }})
        .addRouter({"sureice", sureice::Method::GET_POST,
                    [](auto &req, auto &w, auto sresp) {
                        std::shared_ptr<sureice::response> resp =
                            std::make_shared<sureice::response>();
                        resp->addHeader("Content-Type:",
                                        "text/html; charset=UTF-8");
                        resp->addHeader("Referrer-Policy", "no-referrer");
                        resp->setBody("这是一个自定义的多级路由2");
                        sresp(resp);
                    }});
    web->addRouter(secRouter);
    web->startup(port);
    std::cout << "hello world !" << std::endl;
}
