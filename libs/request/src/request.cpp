#include "request.hpp"

void HttpRequest::setMethod(HttpMethod method) noexcept
{
    method_ = method;
}

void HttpRequest::setTarget(std::string target)
{
    target_ = std ::move(target);
}

void HttpRequest::setProtocol(std::string protocol)
{
    protocol_ = std::move(protocol);
}

void HttpRequest::setHeaders(headers_t headers)
{
    headers_ = std::move(headers);
}

void HttpRequest::setBody(std::string body)
{
    body_ = std::move(body);
}

HttpMethod HttpRequest::getMethod() const noexcept
{
    return method_;
}

const std::string& HttpRequest::getTarget() const noexcept
{
    return target_;
}

const std::string& HttpRequest::getProtocol() const noexcept
{
    return protocol_;
}

auto HttpRequest::getHeaders() const noexcept -> const headers_t&
{
    return headers_;
}

const std::string& HttpRequest::getBody() const noexcept
{
    return body_;
}
