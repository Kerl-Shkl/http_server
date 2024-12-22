#include "request.hpp"
#include "response.hpp"
#include "server.hpp"
#include <iostream>

int main()
{
    try {
        auto frontend_service = std::make_shared<FrontendService>();
        LogicalController logical_controller{frontend_service};
        logical_controller.addAction(  //
            HttpMethod::GET, "/home", [frontend_service](const HttpRequest& request) -> HttpResponse {
                HttpResponse response;
                response.setStatus("OK");
                response.setCode(200);
                auto [content_type, content] = frontend_service->getContent("cv.html");
                response.setBody(std::move(content_type), std::move(content));
                return response;
            });
        logical_controller.addAction(  //
            HttpMethod::GET, "/faq", [frontend_service](const HttpRequest& request) -> HttpResponse {
                HttpResponse response;
                response.setStatus("OK");
                response.setCode(200);
                auto [content_type, content] = frontend_service->getContent("faq.html");
                response.setBody(std::move(content_type), std::move(content));
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
