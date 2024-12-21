#include "common_responses.hpp"

namespace common_response {

HttpResponse notFound()

{
    HttpResponse response;
    response.setCode(404);
    response.setStatus("Not found");
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
        "<p>There is nothing here</p>"
        "</body>"
        "</html>";

    response.setBody(std::string(body));

    return response;
}

}  // namespace common_response
