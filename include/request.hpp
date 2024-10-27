#pragma once

#include <string>

class Request
{
public:
    Request() = default;
    Request(std::string req_body);
    const std::string& getBody() const;
    void complete(std::string_view new_info);
    void reset() noexcept;
    bool isComplete() const noexcept;

private:

    std::string body;
};
