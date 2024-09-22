#include "method_parser.hpp"
#include <algorithm>

auto MethodParser::parse(const string_view str) -> MethodParser::Answer
{
    Answer result;
    for (const auto& [name, method] : methods) {
        auto [str_miss, name_miss] =
            std::mismatch(str.begin(), str.end(), name.begin(), name.end());
        if (areEqueal(str, name, str_miss, name_miss)) {
            return Answer{.need_more = false,
                          .method = method,
                          .method_end = getMethodEnd(str, str_miss)};
        }
        if (isPrefix(str, name, str_miss, name_miss)) {
            return Answer{.need_more = true};
        }
    }
    return Answer{.need_more = false, .method = HttpMethod::INCORRECT};
}

bool MethodParser::areEqueal(const string_view str,
                             const string_view method_name,
                             string_view::const_iterator str_miss,
                             string_view::const_iterator method_miss)
{
    return method_miss == method_name.end() &&
           (str_miss == str.end() || *str_miss == ' ');
}

bool MethodParser::isPrefix(const string_view str,
                            const string_view method_name,
                            string_view::const_iterator str_miss,
                            string_view::const_iterator method_miss)
{
    return str_miss == str.end() && method_miss != method_name.end();
}

auto MethodParser::getMethodEnd(
    const string_view str, string_view::const_iterator str_miss) -> size_type
{
    return str_miss == str.end() ?
               string_view::npos :
               static_cast<size_type>(std::distance(str.begin(), str_miss));
}
