#include "request_builder.hpp"
#include <stdexcept>

void RequestBuilder::complete(std::string_view new_info)
{
    buffer += new_info;
}

void RequestBuilder::processNext()
{
    switch (current_component) {
    case CurrentComponent::method:
        parseMethod();
        break;
    }
}

void RequestBuilder::parseMethod()
{
    auto answer = method_parser.parse(buffer);
    if (answer.need_more) {
        return;
    }
    if (answer.method == HttpMethod::INCORRECT) {
        throw std::runtime_error{"incorrect method"};
    }
    request.setMethod(answer.method);
    actual_pos = answer.method_end;
}
