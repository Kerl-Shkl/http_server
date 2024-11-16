#include "logical_controller.hpp"
#include "common_responses.hpp"

void LogicalController::addAction(HttpMethod method, std::string target, handler_t handler)
{
    auto& target_map = handlers[method];
    target_map[std::move(target)] = std::move(handler);
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] HttpResponse LogicalController::process(HttpRequest request)
{
    HttpMethod method = request.getMethod();
    const std::string& target = request.getTarget();
    auto& target_map = handlers[method];
    auto iter = target_map.find(target);
    if (iter == target_map.end()) {
        return common_response::notFound();
    }
    return iter->second(std::move(request));
}
