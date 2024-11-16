#include "common_responses.hpp"

namespace common_response {

HttpResponse notFound()

{
    HttpResponse response;
    response.setProtocol("HTTP/1.1");
    response.setCode(404);
    response.setStatus("Not found");
    response.addHeader("Server", "Kerl production");
    response.addHeader("Content-Type", "text/html; charset=UTF-8");

    constexpr std::string_view body =
        "<!doctype html>"
        "<html lang=\"en-US\">"
        "<head>"
        "<title> 404 not found </title>"
        "</head>"
        "<body>"
        "<header>"
        "<h1>Not found</h1>"
        "</header>"
        "<p>Теперь ты веришь, мистер Буби?</p>"
        "</body>"
        "</html>";

    response.setBody(std::string(body));

    return response;
}

}  // namespace common_response
