#include "headers_builder.hpp"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <stdexcept>

void HeadersBuilder::add(const string_view str)
{
    assert(actual_part != ActualPart::table_complete);
    for (actual_pos = 0; str.size() > actual_pos;) {
        std::optional<bool> headers_end = headersEnd(str);
        if (!headers_end.has_value()) {
            return;
        }
        if (*headers_end) {
            actual_part = ActualPart::table_complete;
            return;
        }
        if (actual_part == ActualPart::key) {
            const bool key_completed = readKey(str);
            if (key_completed) {
                actual_part = ActualPart::value;
            }
        }
        else { /* actual_part == ActualPart::value */
            const bool value_complete = readValue(str);
            if (value_complete) {
                headers.insert({std::move(key), std::move(value)});
                key.clear();
                value.clear();
                actual_part = ActualPart::key;
            }
        }
    }
}

bool HeadersBuilder::isComplete() const
{
    return actual_part == ActualPart::table_complete;
}

auto HeadersBuilder::getHeaders() const -> const HeadersTable&
{
    return headers;
}

auto HeadersBuilder::getHeaders() -> HeadersTable&
{
    return headers;
}

auto HeadersBuilder::getAfterHeadersPos() const -> size_type
{
    return actual_pos;
}

void HeadersBuilder::reset()
{
    headers.clear();
    key.clear();
    value.clear();
    actual_pos = 0;
    actual_part = ActualPart::key;
}

bool HeadersBuilder::readKey(const string_view str)
{
    if (key.empty()) {
        actual_pos = str.find_first_not_of(' ', actual_pos);
    }
    for (; actual_pos < str.size(); ++actual_pos) {
        char ch = str[actual_pos];
        if (ch == ':') {
            ++actual_pos;  // skip ':'
            boost::trim_right(key);
            return true;
        }
        if (ch == '\r' || ch == '\n') {  // TODO add more forbidden symbols
            throw std::runtime_error{"incorrect header key"};
        }
        key += ch;
        // TODO add check if buffer is too big
    }
    return false;
}

bool HeadersBuilder::readValue(const string_view str)
{
    std::optional<bool> value_end = handleValueEnd(str);
    if (!value_end.has_value()) {
        actual_pos = str.size();
        return false;
    }
    if (*value_end) {
        boost::trim_right(value);
        return true;
    }

    if (value.empty()) {
        actual_pos = str.find_first_not_of(' ', actual_pos);
    }

    for (; actual_pos < str.size(); ++actual_pos) {
        char ch = str[actual_pos];
        if (ch == '\r') {  // TODO add more forbidden symbols
            if (actual_pos + 2 >= str.size()) {
                value_prev_end = str.substr(actual_pos);
                assert(value_prev_end.size() <= 2);
                actual_pos = str.size();
                return false;
            }
            if (str[actual_pos + 1] == '\n' && str[actual_pos + 2] != ' ' && str[actual_pos + 2] != '\t') {
                actual_pos += 2;  // skip '\r\n'
                boost::trim_right(value);
                return true;
            }
        }
        // TODO add check if buffer is too big
        value += ch;
    }
    return false;
}

std::optional<bool> HeadersBuilder::handleValueEnd(const string_view str)
{
    if (!value_prev_end.empty()) {
        if (value_prev_end.size() + str.size() < 3) [[unlikely]] {
            value_prev_end += str;
            return std::nullopt;
        }
        else {
            if (value_prev_end.size() == 1) {
                if (str[0] == '\n' && str[1] != ' ' && str[1] != '\t') {
                    ++actual_pos;
                    value_prev_end.clear();
                    return true;
                }
                value += value_prev_end;
                value_prev_end.clear();
                return false;
            }
            if (value_prev_end[1] == '\n' && str[0] != ' ' && str[0] != '\t') {
                value_prev_end.clear();
                return true;
            }
            value += value_prev_end;
            value_prev_end.clear();
            return false;
        }
    }
    return false;
}

std::optional<bool> HeadersBuilder::headersEnd(const string_view str)
{
    if (headers_may_end) {
        if (str[0] == '\n') {
            headers_may_end = false;
            return true;
        }
        headers_may_end = false;
        throw std::runtime_error{"incorrect headers structure. Key start with \\r"};
    }

    if (actual_part == ActualPart::key && key.empty() && str[actual_pos] == '\r') {
        if (actual_pos + 1 >= str.size()) {
            headers_may_end = true;
            return std::nullopt;
        }
        if (str[actual_pos + 1] == '\n') {
            actual_pos += 2;  // skip \r\n
            return true;
        }
    }
    return false;
}
