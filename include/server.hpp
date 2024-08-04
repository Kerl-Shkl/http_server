#pragma once

#include "connection.hpp"
#include <sys/epoll.h>
#include <unordered_map>

class Server
{
public:
    Server();
    void run();

private:
    void createListenSocket();
    void addListenToEpoll();
    int acceptConnection();
    void addConnection();
    void createEpoll();

    static constexpr short port = 8000;
    int listen_sd = 0;
    int epoll_fd = 0;
    epoll_event event;
    std::unordered_map<int, Connection> connections;
};
