#pragma once

#include <string>

class Response
{
public:
    Response() = default;
    Response(std::string body);
    void complete(std::string_view new_info);
    const std::string& getBody() const noexcept;
    bool isComplete() const noexcept;
    void reset() noexcept;

private:
    std::string body;
};
