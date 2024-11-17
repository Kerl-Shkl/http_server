#pragma once

#include <array>
#include <string_view>

enum class HttpMethod
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,

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

    static Answer parse(const string_view str);

private:
    static bool areEqueal(const string_view str, const string_view method_name,
                          string_view::const_iterator str_miss, string_view::const_iterator method_miss);

    static bool isPrefix(const string_view str, const string_view method_name,
                         string_view::const_iterator str_miss, string_view::const_iterator method_miss);

    static size_type getMethodEnd(const string_view str, string_view::const_iterator str_miss);

    using MethodsTable = std::array<std::pair<string_view, HttpMethod>, 9>;
    static constexpr MethodsTable methods{
        {
         {"GET", HttpMethod::GET},
         {"HEAD", HttpMethod::HEAD},
         {"POST", HttpMethod::POST},
         {"PUT", HttpMethod::PUT},
         {"DELETE", HttpMethod::DELETE},
         {"CONNECT", HttpMethod::CONNECT},
         {"OPTIONS", HttpMethod::OPTIONS},
         {"TRACE", HttpMethod::TRACE},
         {"PATCH", HttpMethod::PATCH},
         }
    };
};
