#pragma once

#include "request.hpp"
#include "response.hpp"
#include <functional>
#include <map>

class LogicalController
{
public:
    using handler_t = std::function<HttpResponse(HttpRequest)>;
    void addAction(HttpMethod method, std::string target, handler_t handler);

    [[nodiscard]] HttpResponse process(HttpRequest request);

private:
    using handlers_seq = std::map<std::string, handler_t>;
    std::unordered_map<HttpMethod, handlers_seq> handlers;
};
