#pragma once

#include "request.hpp"
#include <string>

#include "headers_builder.hpp"

class RequestBuilder
{
public:
    RequestBuilder() = default;
    void complete(std::string_view new_info);
    bool isComplete() const noexcept;

private:
    void processNext();
    void parseFirstLine();
    void parseHeaders();
    void parseBody();

    bool needBody(const HeadersBuilder::HeadersTable& headers) const;

    enum class CurrentComponent
    {
        first_line,
        headers,
        body,
        complete
    };

    CurrentComponent current_component{CurrentComponent::first_line};

    HttpRequest request;
    std::string buffer{};
    std::size_t actual_pos{0};

    HeadersBuilder headers_builder;
};
