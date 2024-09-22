#pragma once

#include <iostream>

class Logger
{
public:
    Logger(std::string pref, std::ostream& out = std::cout);

    void log(std::string_view str) const noexcept;
    void log(std::string_view str, const char *s, int length) const noexcept;
    void log(std::string_view str, const std::string& value) const noexcept;
    template<typename T>
    void log(std::string_view str, T value) const noexcept;

private:
    std::string prefix = "";
    std::ostream& out;
};

template<typename T>
void Logger::log(std::string_view str, T value) const noexcept
{
    out << "[" << prefix << "] " << str << " " << value << std::endl;
}