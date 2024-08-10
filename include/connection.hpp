#pragma once

#include "request.hpp"
#include "response.hpp"
#include <queue>

class Connection
{
public:
    explicit Connection(int sock);
    void readRequest();
    void solveRequest();
    bool writeReady() const noexcept;

private:
    std::queue<Request> requests;
    int socket;
};
