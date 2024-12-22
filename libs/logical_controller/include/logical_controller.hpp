#pragma once

#include "frontend_service.hpp"
#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"
#include <functional>
#include <map>

class LogicalController
{
public:
    using handler_t = std::function<HttpResponse(HttpRequest)>;

    LogicalController(std::shared_ptr<FrontendService> frontend_service);
    void addAction(HttpMethod method, std::string target, handler_t handler);

    [[nodiscard]] HttpResponse process(HttpRequest request) const noexcept;

private:
    [[nodiscard]] HttpResponse doProcess(HttpRequest request) const noexcept;
    HttpResponse contentResponse(std::string&& content_type, std::string&& content) const;
    void addCommonInfo(HttpResponse& response) const noexcept;

    using handlers_seq = std::map<std::string, handler_t>;
    std::unordered_map<HttpMethod, handlers_seq> handlers;
    std::shared_ptr<FrontendService> frontend_service;

    Logger log{"logical_controller"};
};
