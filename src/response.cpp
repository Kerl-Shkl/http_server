#include "response.hpp"

Response::Response(std::string b)
: body(std::move(b))
{}

void Response::complete(std::string_view new_info)
{
    body += new_info;
}

const std::string& Response::getBody() const noexcept
{
    return body;
}

bool Response::isComplete() const noexcept
{
    return !body.empty();
}

void Response::reset() noexcept
{
    body.clear();
}
