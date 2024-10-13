#pragma once

#include "method_parser.hpp"
#include "string"
#include "string_view"
#include <stdexcept>

class FirstLineParser
{
public:
    bool parse(const std::string_view str);
    HttpMethod getMethod() const;
    std::string getTarget() const;
    std::string getProtocol() const;

private:
    size_t lineEnd(const std::string_view str);
    size_t parseMethod(const std::string_view str);
    size_t parseTarget(const std::string_view str);
    size_t parseProtocol(const std::string_view str);

    HttpMethod method{HttpMethod::INCORRECT};
    std::string target{};
    std::string protocol{};
};

struct IncorrectMethod : std::exception
{
    virtual const char *what() const throw() override
    {
        return "Incorrect method";
    }
};

struct IncorrectFirstLine : std::runtime_error
{
    IncorrectFirstLine(std::string message)
    : runtime_error{std::string{"Incorrect first line. "} + message}
    {}

    ~IncorrectFirstLine() override
    {}

    virtual const char *what() const throw() override
    {
        return runtime_error::what();
    }
};
