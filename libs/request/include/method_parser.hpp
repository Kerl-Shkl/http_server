#pragma once

#include <algorithm>
#include <array>
#include <string_view>

enum class HttpMethod
{
    GET,
    PUT,

    INCORRECT
};

class MethodParser
{
public:
    using string_view = std::string_view;
    using size_type = string_view::size_type;

    struct Answer
    {
        bool need_more{false};
        HttpMethod method{HttpMethod::INCORRECT};
        size_type method_end = string_view::npos;
    };

    static Answer parse(const string_view str)
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

private:
    static const bool areEqueal(const string_view str,
                                const string_view method_name,
                                string_view::const_iterator str_miss,
                                string_view::const_iterator method_miss)
    {
        return method_miss == method_name.end() &&
               (str_miss == str.end() || *str_miss == ' ');
    }

    static const bool isPrefix(const string_view str,
                               const string_view method_name,
                               string_view::const_iterator str_miss,
                               string_view::const_iterator method_miss)
    {
        return str_miss == str.end() && method_miss != method_name.end();
    }

    static const size_type getMethodEnd(const string_view str,
                                        string_view::const_iterator str_miss)
    {
        return str_miss == str.end() ?
                   string_view::npos :
                   static_cast<size_type>(std::distance(str.begin(), str_miss));
    }

    using MethodsTable = std::array<std::pair<string_view, HttpMethod>, 2>;
    static constexpr MethodsTable methods{
        {
         {"GET", HttpMethod::GET},
         {"PUT", HttpMethod::PUT},
         }
    };
};
