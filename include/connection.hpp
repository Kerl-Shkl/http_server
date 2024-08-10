#pragma once

#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"

class Connection
{
public:
    explicit Connection(int sock);
    void readRequest();
    void solveRequest();
    void writeRequest();
    bool writeReady() const noexcept;

private:
    int socket;
    Request actual_request;
    Response response;
    Logger log{"connection"};
};
