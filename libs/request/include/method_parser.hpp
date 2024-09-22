#pragma once

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

    static Answer parse(const string_view str);

private:
    static bool areEqueal(const string_view str, const string_view method_name,
                          string_view::const_iterator str_miss,
                          string_view::const_iterator method_miss);

    static bool isPrefix(const string_view str, const string_view method_name,
                         string_view::const_iterator str_miss,
                         string_view::const_iterator method_miss);

    static size_type getMethodEnd(const string_view str,
                                  string_view::const_iterator str_miss);

    using MethodsTable = std::array<std::pair<string_view, HttpMethod>, 2>;
    static constexpr MethodsTable methods{
        {
         {"GET", HttpMethod::GET},
         {"PUT", HttpMethod::PUT},
         }
    };
};
