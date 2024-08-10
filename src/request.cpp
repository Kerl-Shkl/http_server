#include "request.hpp"

Request::Request(std::string req_body)
: body(std::move(req_body))
{}

const std::string& Request::getBody() const
{
    return body;
}

void Request::complete(std::string_view new_info)
{
    body += new_info;
}

void Request::reset() noexcept
{
    body.clear();
}

bool Request::isComplete() const noexcept
{
    return !body.empty();
}
