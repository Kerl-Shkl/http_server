#pragma once

#include "request.hpp"
#include <string>

class RequestBuilder
{
public:
    RequestBuilder() = default;
    HttpRequest build();
    void complete(std::string_view new_info);
    bool isComplete() const noexcept;

private:
    enum class CurrentComponent
    {
        method,
        target,
        protocol,
        headers,
        body,
        complete
    };

    CurrentComponent current_component{CurrentComponent::method};
    bool need_body;

    HttpRequest request;
    std::string buffer;
};
