#include <logger.hpp>

Logger::Logger(std::string pref, std::ostream& o)
: prefix(pref)
, out(o)
{}

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
