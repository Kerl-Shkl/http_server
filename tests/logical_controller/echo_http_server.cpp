#include "backend_interface.hpp"
#include "server.hpp"

struct MockBackend : public BackendInterface
{
    void init(std::shared_ptr<LogicalController> controller) override
    {
        controller->addAction(HttpMethod::GET, "/home", [](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            response.setStatus("OK");
            response.setCode(200);
            response.setBody("", request.getBody());
            for (const auto& [key, value] : request.getHeaders()) {
                response.addHeader(key, value);
            }
            return response;
        });
    }
};

int main()
{
    try {
        Server server{"", 8678};
        auto backend = std::make_shared<MockBackend>();
        server.setBackendService(backend);
        server.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
