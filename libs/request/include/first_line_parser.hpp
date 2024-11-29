#pragma once

#include "method_parser.hpp"
#include "string"
#include "string_view"
#include <stdexcept>
#include <unordered_map>

class FirstLineParser
{
public:
    bool parse(const std::string_view str);
    [[nodiscard]] HttpMethod getMethod() const;
    [[nodiscard]] std::string& getTarget();
    [[nodiscard]] std::string& getProtocol();
    [[nodiscard]] size_t getLineEnd() const;
    [[nodiscard]] std::unordered_map<std::string, std::string>& getParameters();
    [[nodiscard]] std::unordered_map<std::string, std::string>& getQueries();
    [[nodiscard]] std::string& getFragment();

private:
    size_t lineEnd(const std::string_view str);
    size_t parseMethod(const std::string_view str);
    size_t parseTarget(const std::string_view str);
    size_t parseProtocol(const std::string_view str);

    size_t cutParameter(const std::string_view str, size_t start);
    size_t cutQuery(const std::string_view str, size_t start);
    size_t cutFragment(const std::string_view str, size_t start);

    HttpMethod method{HttpMethod::INCORRECT};
    std::string target;
    std::string protocol;
    size_t line_end = std::string_view::npos;
    std::unordered_map<std::string, std::string> parameters;
    std::unordered_map<std::string, std::string> queries;
    std::string fragment;
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
