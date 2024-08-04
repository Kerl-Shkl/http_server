#include "request.hpp"

Request::Request(std::string req_body)
: body(std::move(req_body))
{}

const std::string& Request::getBody() const
{
    return body;
}
