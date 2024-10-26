#pragma once

#include "method_parser.hpp"
#include "string"
#include "string_view"
#include <stdexcept>

class FirstLineParser
{
public:
    bool parse(const std::string_view str);
    [[nodiscard]] HttpMethod getMethod() const;
    [[nodiscard]] std::string getTarget() const;
    [[nodiscard]] std::string getProtocol() const;
    [[nodiscard]] size_t getLineEnd() const;

private:
    size_t lineEnd(const std::string_view str);
    size_t parseMethod(const std::string_view str);
    size_t parseTarget(const std::string_view str);
    size_t parseProtocol(const std::string_view str);

    HttpMethod method{HttpMethod::INCORRECT};
    std::string target;
    std::string protocol;
    size_t line_end = std::string_view::npos;
};

struct IncorrectMethod : std::exception
{
    const char *what() const throw() override  // NOLINT
    {
        return "Incorrect method";
    }
};

struct IncorrectFirstLine : std::runtime_error
{
    IncorrectFirstLine(std::string message)
    : runtime_error{std::string{"Incorrect first line. "} + message}
    {}

    ~IncorrectFirstLine() override = default;

    const char *what() const throw() override  // NOLINT
    {
        return runtime_error::what();
    }
};
