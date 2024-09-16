#pragma once

#include "method_parser.hpp"
#include <string>
#include <unordered_map>

/*
TODO:
    1) replace std::string with std::string_view
*/

class HttpRequest
{
public:
    HttpRequest() = default;

    void setMethod(HttpMethod method) noexcept;
    void setTarget(std::string target);
    void setProtocol(std::string protocol);
    void addHeader(std::string key, std::string value);
    void setBody(std::string body);

private:
    using headers_t = std::unordered_map<std::string, std::string>;

    HttpMethod method_;
    std::string target_;
    std::string protocol_;
    headers_t headers_;
    std::string body_;
};
