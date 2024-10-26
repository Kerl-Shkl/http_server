#include "request_builder.hpp"
#include "first_line_parser.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
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
    CurrentComponent old_state{};
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

namespace {
inline std::string::difference_type toDiffType(size_t pos)
{
    return boost::numeric_cast<std::string::difference_type>(pos);
}
}  // namespace

void RequestBuilder::parseHeaders()
{
    if (actual_pos >= buffer.size()) {
        return;
    }
    headers_builder.add(std::string_view{buffer.begin() + toDiffType(actual_pos), buffer.end()});

    if (!headers_builder.isComplete()) {
        actual_pos = buffer.size();
        return;
    }

    const auto& headers = headers_builder.getHeaders();
    request.setHeaders(headers);
    actual_pos += headers_builder.getAfterHeadersPos();
    current_component = needBody() ? CurrentComponent::body : CurrentComponent::complete;
}

bool RequestBuilder::needBody() const
{
    // TODO add check method allow body
    const auto& headers = request.getHeaders();
    return headers.contains("Content-Length");
}

void RequestBuilder::parseBody()
{
    const auto& headers = request.getHeaders();
    assert(headers.contains("Content-Length"));
    auto body_length = boost::lexical_cast<size_t>(headers.at("Content-Length"));

    if (actual_pos + body_length > buffer.size()) {
        return;
    }

    auto body_start = buffer.begin() + toDiffType(actual_pos);
    auto body_end = body_start + toDiffType(body_length);
    request.setBody(std::string{body_start, body_end});
    current_component = CurrentComponent::complete;
}
