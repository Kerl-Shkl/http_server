#include "request_builder.hpp"
#include "first_line_parser.hpp"
#include <cassert>

bool RequestBuilder::isComplete() const noexcept
{
    return current_component == CurrentComponent::complete;
}

HttpRequest& RequestBuilder::getRequest() noexcept
{
    return request;
}

const HttpRequest& RequestBuilder::getRequest() const noexcept
{
    return request;
}

void RequestBuilder::complete(std::string_view new_info)
{
    buffer += new_info;
    parse();
}

void RequestBuilder::parse()
{
    CurrentComponent old_state;
    do {
        old_state = current_component;
        processNext();
    } while (old_state != current_component);
}

void RequestBuilder::processNext()
{
    switch (current_component) {
    case CurrentComponent::first_line:
        parseFirstLine();
        break;
    case CurrentComponent::headers:
        parseHeaders();
        break;
    case CurrentComponent::body:
        parseBody();
        break;
    case CurrentComponent::complete:
        // TODO complete
        break;
    default:
        assert(false);
    }
}

void RequestBuilder::parseFirstLine()
{
    assert(actual_pos == 0);
    FirstLineParser first_line_parser;
    const bool first_line_complete = first_line_parser.parse(buffer);
    if (!first_line_complete) {
        return;
    }

    request.setMethod(first_line_parser.getMethod());
    request.setTarget(first_line_parser.getTarget());
    request.setProtocol(first_line_parser.getProtocol());

    actual_pos = first_line_parser.getLineEnd();
    current_component = CurrentComponent::headers;
}

void RequestBuilder::parseHeaders()
{
    if (actual_pos >= buffer.size()) {
        return;
    }
    headers_builder.add(std::string_view{buffer.begin() + actual_pos, buffer.end()});

    if (!headers_builder.isComplete()) {
        actual_pos = buffer.size();
        return;
    }

    const auto& headers = headers_builder.getHeaders();
    request.setHeaders(headers);
    actual_pos += headers_builder.getAfterHeadersPos();
    current_component = needBody(headers) ? CurrentComponent::body : CurrentComponent::complete;
}

bool RequestBuilder::needBody(const HeadersBuilder::HeadersTable& headers) const
{
    return false;  // TODO ASAP
}

void RequestBuilder::parseBody()
{
    ;
}
