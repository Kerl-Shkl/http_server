#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

class HeadersBuilder
{
public:
    using HeadersTable = std::unordered_map<std::string, std::string>;
    using string_view = std::string_view;
    using size_type = string_view::size_type;

    void add(const string_view str);
    bool isComplete() const;
    const HeadersTable& getHeaders() const;
    HeadersTable& getHeaders();
    size_type getAfterHeadersPos() const;
    void reset();

private:
    bool readKey(const string_view str);
    bool readValue(const string_view str);
    bool headersEnd(const string_view str);

    enum class ActualPart
    {
        key,
        value,
        table_complete
    };

    HeadersTable headers{};
    std::string key{};
    std::string value{};
    size_type actual_pos{0};
    ActualPart actual_part{ActualPart::key};
};
