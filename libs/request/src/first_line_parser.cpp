#include "first_line_parser.hpp"
#include "method_parser.hpp"

HttpMethod FirstLineParser::getMethod() const
{
    return method;
}

std::string FirstLineParser::getTarget() const
{
    return target;
}

std::string FirstLineParser::getProtocol() const
{
    return protocol;
}

size_t FirstLineParser::getLineEnd() const
{
    return line_end == std::string_view::npos ? line_end : line_end + 1;  // plus one for '\n'
}

bool FirstLineParser::parse(const std::string_view str)
{
    line_end = lineEnd(str);
    if (line_end == std::string_view::npos) {
        return false;
    }
    std::string_view cutted_str{str.data(), line_end + 1};
    size_t method_end = parseMethod(cutted_str);

    // TODO may be it is better to use trim
    const size_t target_start = method_end + 1;
    if (target_start >= cutted_str.size()) {
        throw IncorrectFirstLine{"There is no target"};
    }
    cutted_str = std::string_view{cutted_str.begin() + target_start, cutted_str.end()};
    size_t target_end = parseTarget(cutted_str);

    const size_t protocol_start = target_end + 1;
    if (protocol_start >= cutted_str.size()) {
        throw IncorrectFirstLine{"There is no protocol"};
    }
    cutted_str = std::string_view{cutted_str.begin() + protocol_start, cutted_str.end()};
    parseProtocol(cutted_str);
    // TODO check if line ends after protocol
    return true;
}

size_t FirstLineParser::lineEnd(const std::string_view str)
{
    return str.find_first_of('\n');
}

size_t FirstLineParser::parseMethod(const std::string_view str)
{
    auto answer = MethodParser::parse(str);
    if (answer.need_more) {
        throw IncorrectFirstLine{"first line unexpectedly ends"};
    }
    if (answer.method == HttpMethod::INCORRECT) {
        throw IncorrectMethod{};
    }
    method = answer.method;
    return answer.method_end;
}

size_t FirstLineParser::parseTarget(const std::string_view str)
{
    auto target_end = str.find_first_of(' ');
    if (target_end == std::string_view::npos) {
        throw IncorrectFirstLine{"There is no end of target"};
    }
    if (target_end == 0) {
        throw IncorrectFirstLine{"There is no target"};
    }
    target = std::string{str.begin(), str.begin() + target_end};
    return target_end;
}

size_t FirstLineParser::parseProtocol(const std::string_view str)
{
    auto protocol_end = str.find_first_of(" \n");
    if (protocol_end == std::string_view::npos) {
        throw IncorrectFirstLine{"There is not end of protocol"};
    }
    if (protocol_end == 0) {
        throw IncorrectFirstLine{"There is no protocol"};
    }
    protocol = std::string{str.begin(), str.begin() + protocol_end};
    return protocol_end;
}
