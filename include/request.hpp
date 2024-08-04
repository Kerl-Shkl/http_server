#pragma once

#include <string>

class Request
{
public:
    Request(std::string req_body);
    const std::string& getBody() const;

private:
    std::string body;
};
