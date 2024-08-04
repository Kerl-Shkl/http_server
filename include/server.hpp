#pragma once

#include "connection.hpp"

class Server
{
public:
    Server();
    void run();

private:
    void createListenSocket();
    Connection acceptConnection();

    static constexpr short port = 8000;
    int listen_sd = 0;
};
