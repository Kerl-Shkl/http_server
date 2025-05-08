#include "logical_controller.hpp"
#include "common_responses.hpp"
#include <cassert>

LogicalController::LogicalController()
{
    handlers[HttpMethod::GET] = {};
    handlers[HttpMethod::HEAD] = {};
    handlers[HttpMethod::POST] = {};
    handlers[HttpMethod::PUT] = {};
    handlers[HttpMethod::DELETE] = {};
    handlers[HttpMethod::CONNECT] = {};
    handlers[HttpMethod::OPTIONS] = {};
    handlers[HttpMethod::TRACE] = {};
    handlers[HttpMethod::PATCH] = {};
}

void LogicalController::setFrontendService(std::shared_ptr<FrontendService> frontend)
{
    frontend_service = std::move(frontend);
}

void LogicalController::addAction(HttpMethod method, std::string target, handler_t handler)
{
    auto& target_map = handlers[method];
    target_map[std::move(target)] = std::move(handler);
}

[[nodiscard]] HttpResponse LogicalController::process(HttpRequest request) const noexcept
{
    HttpResponse response = doProcess(std::move(request));
    addCommonInfo(response);
    return response;
}

HttpResponse LogicalController::doProcess(HttpRequest request) const noexcept
{
    HttpMethod method = request.getMethod();
    const std::string& target = request.getTarget();
    assert(handlers.contains(method));
    const auto& target_map = handlers.at(method);
    if (auto iter = target_map.find(target); iter != target_map.end()) {
        return iter->second(std::move(request));
    }
    log.log("No such target: " + target);
    auto [content_type, content] = frontend_service->getContent(target);
    if (!content.empty()) {
        return contentResponse(std::move(content_type), std::move(content));
    }
    log.log("No such content: " + target);
    return common_response::notFound();
}

HttpResponse LogicalController::contentResponse(std::string&& content_type, std::string&& content) const
{
    HttpResponse response;
    response.setStatus("OK");
    response.setCode(200);
    response.setBody(std::move(content_type), std::move(content));
    return response;
}

void LogicalController::addCommonInfo(HttpResponse& response) const noexcept
{
    response.setProtocol("HTTP/1.1");
    response.addHeader("Server", "Kerl production");
}
