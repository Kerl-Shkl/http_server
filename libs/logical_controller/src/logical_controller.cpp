#include "logical_controller.hpp"

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] auto LogicalController::process(HttpRequest request) -> HttpResponse
{
    using namespace std::string_literals;
    std::string result;
    result += std::to_string(static_cast<int>(request.getMethod()));
    result += " "s + request.getTarget() + " "s + request.getProtocol() + "\n"s;
    for (const auto& [key, value] : request.getHeaders()) {
        result += key + " : "s + value + "\n";  // NOLINT
    }
    result += "\n"s + request.getBody();

    return result;
}
