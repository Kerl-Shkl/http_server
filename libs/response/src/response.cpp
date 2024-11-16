#include "response.hpp"
#include <sstream>

void HttpResponse::setProtocol(std::string new_protocol)
{
    protocol = std::move(new_protocol);
}

void HttpResponse::setCode(uint64_t new_code)
{
    code = new_code;
}

void HttpResponse::setStatus(std::string new_status)
{
    status = std::move(new_status);
}

void HttpResponse::addHeader(std::string key, std::string value)
{
    headers.try_emplace(std::move(key), std::move(value));
}
void HttpResponse::setBody(std::string new_body)
{
    body = std::move(new_body);
}

std::string HttpResponse::buildMessage() const
{
    std::stringstream result;
    result << protocol << " " << code << " " << status;
    if (!status.ends_with("\r\n")) {
        result << "\r\n";
    }
    for (const auto& [key, value] : headers) {
        result << key << ": " << value << "\r\n";  // NOLINT(performance-inefficient-string-concatenation)
    }
    result << "\r\n";
    result << body;
    return result.str();
}