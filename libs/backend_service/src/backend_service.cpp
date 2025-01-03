#include "backend_service.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"

BackendService::BackendService()
: controller{std::make_shared<LogicalController>()}
{}

BackendService::~BackendService() = default;

void BackendService::setFrontendService(std::shared_ptr<FrontendService> frontend_service)
{
    frontend = std::move(frontend_service);
    controller->setFrontendService(frontend);
}

void BackendService::init()
{
    controller->addAction(  //
        HttpMethod::GET, "/home", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            response.setStatus("OK");
            response.setCode(200);
            auto [content_type, content] = frontend->getContent("cv.html");
            response.setBody(std::move(content_type), std::move(content));
            return response;
        });
    controller->addAction(  //
        HttpMethod::GET, "/faq", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            response.setStatus("OK");
            response.setCode(200);
            auto [content_type, content] = frontend->getContent("faq.html");
            response.setBody(std::move(content_type), std::move(content));
            return response;
        });
}

std::shared_ptr<LogicalController> BackendService::getLogicalController()
{
    return controller;
}
