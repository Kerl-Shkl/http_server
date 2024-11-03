#include "logical_controller.hpp"

static constexpr std::string_view html_example =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "    <head>\n"
    "        <title>Example</title>\n"
    "    </head>\n"
    "    <body>\n"
    "        <p>This is an example of a simple HTML page with one paragraph.</p>\n"
    "    </body>\n"
    "</html>\n";

static constexpr std::string_view response_example =
    "HTTP/1.1 200 OK\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Server: Kerl-Server\r\n";

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] auto LogicalController::process(HttpRequest request) -> HttpResponse
{
    using namespace std::string_literals;
    // std::string result;
    // result += std::to_string(static_cast<int>(request.getMethod()));
    // result += " "s + request.getTarget() + " "s + request.getProtocol() + "\n"s;
    // for (const auto& [key, value] : request.getHeaders()) {
    //     result += key + " : "s + value + "\n";  // NOLINT
    // }
    // result += "\n"s + request.getBody();
    // return result;

    std::string response = std::string(response_example) + "Content-Length: "s +
                           std::to_string(html_example.size()) + "\r\n\r\n" + std::string(html_example);
    return response;
}
