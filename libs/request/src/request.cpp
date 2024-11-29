#include "request.hpp"

void HttpRequest::setMethod(HttpMethod new_method) noexcept
{
    method = new_method;
}

void HttpRequest::setTarget(std::string new_target)
{
    target = std ::move(new_target);
}

void HttpRequest::setParameters(parameters_t new_parameters)
{
    parameters = std::move(new_parameters);
}

void HttpRequest::setQueries(queries_t new_queries)
{
    queries = std::move(new_queries);
}

void HttpRequest::setProtocol(std::string new_protocol)
{
    protocol = std::move(new_protocol);
}

void HttpRequest::setHeaders(headers_t new_headers)
{
    headers = std::move(new_headers);
}

void HttpRequest::setBody(std::string new_body)
{
    body = std::move(new_body);
}

HttpMethod HttpRequest::getMethod() const noexcept
{
    return method;
}

const std::string& HttpRequest::getTarget() const noexcept
{
    return target;
}

auto HttpRequest::getParameters() const noexcept -> const parameters_t&
{
    return parameters;
}

auto HttpRequest::getQueries() const noexcept -> const queries_t&
{
    return queries;
}

const std::string& HttpRequest::getProtocol() const noexcept
{
    return protocol;
}

auto HttpRequest::getHeaders() const noexcept -> const headers_t&
{
    return headers;
}

const std::string& HttpRequest::getBody() const noexcept
{
    return body;
}
