#include <logger.hpp>

Logger::Logger(std::string pref, std::ostream& o)
: prefix(std::move(pref))
, out(o)
{}

void Logger::updatePrefix(const std::string_view new_prefix)
{
    prefix = new_prefix;
}

void Logger::log(std::string_view str) const noexcept
{
    out << "[" << prefix << "] " << str;
    if (str.back() != '\n') {
        out << std::endl;
    }
}

void Logger::log(std::string_view str, const char *s, int length) const noexcept
{
    out << "[" << prefix << "] " << str << " ";
    out.write(s, length);
    if (s[length - 1] != '\n') {
        out << std::endl;
    }
}

void Logger::log(std::string_view str, const std::string& value) const noexcept
{
    out << "[" << prefix << "] " << str << " " << value;
    if (value.back() != '\n') {
        out << std::endl;
    }
}

void Logger::log(std::string_view str, const std::string_view value, bool message) const noexcept
{
    using pos_t = std::string_view::size_type;
    out << "[" << prefix << "] " << str << std::endl;

    std::string_view right_part{value};
    while (!right_part.empty()) {
        pos_t endline = right_part.find_first_of('\n');
        out << "\t| " << right_part.substr(0, endline) << std::endl;
        right_part = (endline == std::string::npos) ? std::string_view{} : right_part.substr(endline + 1);
    }
}
