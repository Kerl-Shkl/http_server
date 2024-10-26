#pragma once

#include "method_parser.hpp"
#include <string>
#include <unordered_map>

class HttpRequest
{
public:
    using headers_t = std::unordered_map<std::string, std::string>;

    HttpRequest() = default;

    void setMethod(HttpMethod method) noexcept;
    void setTarget(std::string target);
    void setProtocol(std::string protocol);
    void setHeaders(headers_t headers);
    void setBody(std::string body);

    HttpMethod getMethod() const noexcept;
    const std::string& getTarget() const noexcept;
    const std::string& getProtocol() const noexcept;
    const headers_t& getHeaders() const noexcept;
    const std::string& getBody() const noexcept;

private:
    HttpMethod method{HttpMethod::INCORRECT};
    std::string target;
    std::string protocol;
    headers_t headers;
    std::string body;
};
