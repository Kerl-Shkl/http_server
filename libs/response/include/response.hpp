#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

class HttpResponse
{
public:
    using headers_t = std::unordered_map<std::string, std::string>;

    void setCode(uint64_t new_code);
    void setStatus(std::string new_status);
    void addHeader(std::string key, std::string value);
    void setBody(std::string body);

    std::string buildMessage() const;

private:
    void setProtocol(std::string new_protocol);

    std::string protocol;
    uint64_t code;
    std::string status;
    headers_t headers;
    std::string body;

    friend class LogicalController;
};
