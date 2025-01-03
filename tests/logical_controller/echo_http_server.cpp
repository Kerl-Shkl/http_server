#include "server.hpp"

int main()
{
    try {
        auto logical_controller = std::make_shared<LogicalController>();
        logical_controller->addAction(HttpMethod::GET, "/home",
                                      [](const HttpRequest& request) -> HttpResponse {
                                          HttpResponse response;
                                          response.setStatus("OK");
                                          response.setCode(200);
                                          response.setBody("", request.getBody());
                                          for (const auto& [key, value] : request.getHeaders()) {
                                              response.addHeader(key, value);
                                          }
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
