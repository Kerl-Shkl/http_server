#include "response.hpp"

void Response::setStatusLine(std::string new_status_line)
{
    status_line = std::move(new_status_line);
}

void Response::addHeader(std::string key, std::string value)
{
    headers.try_emplace(std::move(key), std::move(value));
}
void Response::setBody(std::string new_body)
{
    body = std::move(new_body);
}

std::string Response::buildMessage() const
{
    std::string result = status_line;
    if (!status_line.ends_with("\r\n")) {
        result += "\r\n";
    }
    for (const auto& [key, value] : headers) {
        result += key + ": " + value + "\r\n";  // NOLINT(performance-inefficient-string-concatenation)
    }
    result += "\r\n";
    result += body;
    return result;
}
