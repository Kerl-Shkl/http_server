#include "server.hpp"

int main()
{
    try {
        LogicalController logical_controller;
        logical_controller.addAction(HttpMethod::GET, "/home",
                                     [](const HttpRequest& request) -> HttpResponse {
                                         HttpResponse response;
                                         response.setStatus("OK");
                                         response.setCode(200);
                                         for (const auto& [key, value] : request.getHeaders()) {
                                             response.addHeader(key, value);
                                         }
                                         response.setBody(request.getBody());
                                         return response;
                                     });
        Server server(std::move(logical_controller), 8678);
        server.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
