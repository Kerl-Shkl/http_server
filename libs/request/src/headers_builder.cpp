#include "headers_builder.hpp"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <stdexcept>

void HeadersBuilder::add(const string_view str)
{
    assert(actual_part != ActualPart::table_complete);
    for (actual_pos = 0; str.size() > actual_pos;) {
        if (headersEnd(str)) {
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

auto HeadersBuilder::getHeaders() const
    -> HeadersTable  // May it worth to move headers
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
        if (ch == '\n') {  // TODO add more forbidden symbols
            throw std::runtime_error{"incorrect header key"};
        }
        key += ch;
        // TODO add check if buffer is too big
    }
    return false;
}

bool HeadersBuilder::readValue(const string_view str)
{
    if (value.empty()) {
        actual_pos = str.find_first_not_of(' ', actual_pos);
    }
    for (; actual_pos < str.size(); ++actual_pos) {
        char ch = str[actual_pos];
        if (ch == '\n') {  // TODO add more forbidden symbols
            ++actual_pos;  // skip '\n'
            boost::trim_right(value);
            return true;
        }
        // TODO add check if buffer is too big
        value += ch;
    }
    return false;
}

bool HeadersBuilder::headersEnd(const string_view str)
{
    if (actual_part == ActualPart::key && key.empty() &&
        str[actual_pos] == '\n')
    {
        ++actual_pos;
        return true;
    }
    return false;
}
