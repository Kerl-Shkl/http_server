#include "first_line_parser.hpp"
#include "method_parser.hpp"

HttpMethod FirstLineParser::getMethod() const
{
    return method;
}

std::string& FirstLineParser::getTarget()
{
    return target;
}

std::string& FirstLineParser::getProtocol()
{
    return protocol;
}

size_t FirstLineParser::getLineEnd() const
{
    return line_end == std::string_view::npos ? line_end : line_end + 1;  // plus one for '\n'
}

std::unordered_map<std::string, std::string>& FirstLineParser::getParameters()
{
    return parameters;
}

std::unordered_map<std::string, std::string>& FirstLineParser::getQueries()
{
    return queries;
}

std::string& FirstLineParser::getFragment()
{
    return fragment;
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
    bool query_section = false;
    for (size_t i = 0; i < str.size(); ++i) {
        const char ch = str[i];
        if (ch == ';') {
            i = cutParameter(str, i);
        }
        else if (ch == '?' || (query_section && ch == '&')) {
            query_section = true;
            i = cutQuery(str, i);
        }
        else if (ch == '#') {
            return cutFragment(str, i);
        }
        else if (ch == ' ') {
            if (i == 0) {
                throw IncorrectFirstLine{"Empty target"};
            }
            return i;
        }
        else {
            if (query_section) {
                throw IncorrectFirstLine{"Unexpected symbol in query section"};
            }
            target += ch;
        }
    }
    throw IncorrectFirstLine{"There is no end of target"};
}

size_t FirstLineParser::cutParameter(const std::string_view str, size_t start)
{
    // TODO add check for prohibited symbols
    const auto param_name_end = str.find('=', start);
    if (param_name_end == std::string_view::npos) {
        throw IncorrectFirstLine{"Incorrect parameter. Where is no = "};
    }
    const std::string parameter_name{str.substr(start + 1, param_name_end - start - 1)};
    const auto param_value_end = str.find_first_of(";?#/ ", param_name_end + 1);
    const auto parameter_value = str.substr(param_name_end + 1, param_value_end - param_name_end - 1);
    parameters[parameter_name] = parameter_value;
    target += ";" + parameter_name;
    return param_value_end - 1;
}

size_t FirstLineParser::cutQuery(const std::string_view str, size_t start)
{
    // TODO add check for prohibited symbols
    const auto query_name_end = str.find('=', start);
    if (query_name_end == std::string_view::npos) {
        throw IncorrectFirstLine{"Incorrect query. Where is no = "};
    }
    const std::string query_name{str.substr(start + 1, query_name_end - start - 1)};
    const auto query_value_end = str.find_first_of("&# ", query_name_end + 1);
    const auto query_value = str.substr(query_name_end + 1, query_value_end - query_name_end - 1);
    queries[query_name] = query_value;
    return query_value_end - 1;
}

size_t FirstLineParser::cutFragment(const std::string_view str, size_t start)
{
    const auto fragment_end = str.find(' ', start + 1);
    if (fragment_end == std::string_view::npos) {
        throw IncorrectFirstLine{"Fragment has no end. (lol)"};
    }
    fragment = str.substr(start + 1, fragment_end - start - 1);
    return fragment_end;
}

size_t FirstLineParser::parseProtocol(const std::string_view str)
{
    auto protocol_end = str.find_first_of(" \r\n");
    if (protocol_end == std::string_view::npos) {
        throw IncorrectFirstLine{"There is not end of protocol"};
    }
    if (protocol_end == 0) {
        throw IncorrectFirstLine{"There is no protocol"};
    }
    protocol = std::string{str.begin(), str.begin() + protocol_end};
    return protocol_end;
}
