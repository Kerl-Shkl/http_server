#include "request.hpp"
#include "response.hpp"
#include "server.hpp"
#include <fstream>
#include <iostream>

int main()
{
    try {
        LogicalController logical_controller;
        logical_controller.addAction(HttpMethod::GET, "/home",
                                     [](const HttpRequest& request) -> HttpResponse {
                                         HttpResponse response;
                                         response.setStatus("OK");
                                         response.setCode(200);
                                         response.addHeader("Content-Type", "text/html; charset=UTF-8");
                                         std::ifstream home_file("html_css/CV/cv.html");
                                         std::string content((std::istreambuf_iterator<char>(home_file)),
                                                             (std::istreambuf_iterator<char>()));
                                         response.setBody(std::move(content));

                                         return response;
                                     });
        logical_controller.addAction(HttpMethod::GET, "/faq", [](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            response.setStatus("OK");
            response.setCode(200);
            response.addHeader("Content-Type", "text/html; charset=UTF-8");
            std::ifstream home_file("html_css/faq.html");
            std::string content((std::istreambuf_iterator<char>(home_file)),
                                (std::istreambuf_iterator<char>()));
            response.setBody(std::move(content));

            return response;
        });
        Server server{std::move(logical_controller)};
        server.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
