#pragma once
#include <string>
#include <unordered_map>

class Response
{
    using headers_t = std::unordered_map<std::string, std::string>;

    void setStatusLine(std::string status_line);
    void addHeader(std::string key, std::string value);
    void setBody(std::string body);

    std::string buildMessage() const;

private:
    std::string status_line;
    headers_t headers;
    std::string body;
};
