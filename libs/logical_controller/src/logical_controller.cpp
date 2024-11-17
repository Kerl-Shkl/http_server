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

void LogicalController::addAction(HttpMethod method, std::string target, handler_t handler)
{
    auto& target_map = handlers[method];
    target_map[std::move(target)] = std::move(handler);
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] HttpResponse LogicalController::process(HttpRequest request) const
{
    HttpResponse response = doProcess(std::move(request));
    addCommonInfo(response);
    return response;
}

HttpResponse LogicalController::doProcess(HttpRequest request) const
{
    HttpMethod method = request.getMethod();
    const std::string& target = request.getTarget();
    assert(handlers.contains(method));
    const auto& target_map = handlers.at(method);
    auto iter = target_map.find(target);
    if (iter == target_map.end()) {
        return common_response::notFound();
    }
    return iter->second(std::move(request));
}

void LogicalController::addCommonInfo(HttpResponse& response) const
{
    response.setProtocol("HTTP/1.1");
    response.addHeader("Server", "Kerl production");
}
