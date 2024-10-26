#pragma once

#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"

class LinuxConnection
{
public:
    explicit LinuxConnection(int sock);
    int getSocket() const noexcept;
    bool closed() const noexcept;
    void closeConnection() noexcept;
    virtual void readRequest();
    virtual void solveRequest();
    virtual void writeResponse();
    bool writeReady() const noexcept;

protected:
    Request actual_request;
    Response response;

private:
    int socket;
    Logger log{"connection"};
};
