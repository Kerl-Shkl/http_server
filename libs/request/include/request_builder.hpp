#pragma once

#include "request.hpp"
#include <string>

#include "headers_builder.hpp"
#include "method_parser.hpp"

class RequestBuilder
{
public:
    RequestBuilder() = default;
    void complete(std::string_view new_info);
    bool isComplete() const noexcept;

private:
    void processNext();
    void parseMethod();
    void parseTarget();
    void parseProtocol();
    void parseHeaders();
    void parseBody();

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
    std::string buffer{};
    std::size_t actual_pos{0};

    MethodParser method_parser;
    HeadersBuilder headers_builder;
};
