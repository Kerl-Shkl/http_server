#pragma once

#include "method_parser.hpp"
#include <string>
#include <unordered_map>

class HttpRequest
{
public:
    using headers_t = std::unordered_map<std::string, std::string>;
    using parameters_t = std::unordered_map<std::string, std::string>;
    using queries_t = std::unordered_map<std::string, std::string>;

    HttpRequest() = default;

    void setMethod(HttpMethod method) noexcept;
    void setTarget(std::string target);
    void setParameters(parameters_t parameters);
    void setQueries(queries_t queries);
    void setProtocol(std::string protocol);
    void setHeaders(headers_t headers);
    void setBody(std::string body);

    HttpMethod getMethod() const noexcept;
    const std::string& getTarget() const noexcept;
    const parameters_t& getParameters() const noexcept;
    const queries_t& getQueries() const noexcept;
    const std::string& getProtocol() const noexcept;
    const headers_t& getHeaders() const noexcept;
    const std::string& getBody() const noexcept;

private:
    HttpMethod method{HttpMethod::INCORRECT};
    std::string target;
    parameters_t parameters;
    queries_t queries;
    std::string protocol;
    headers_t headers;
    std::string body;
};
