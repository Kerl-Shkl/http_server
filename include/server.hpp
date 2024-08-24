#pragma once

#include "connections_list.hpp"
#include "epoll_connection.hpp"
#include "logger.hpp"
#include <sys/epoll.h>
#include <vector>

class Server
{
public:
    Server();
    void run();

private:
    using Connection = EpollConnection;

    void waitConnection();
    bool newClients() const noexcept;
    void returnConnectionToWaiters(Connection& connection);
    void doIO(Connection& connection);

    void createListenSocket();
    void addListenToEpoll();
    int acceptConnection();
    void addConnection();
    void createEpoll();

    static constexpr short port = 8000;
    int listen_sd = 0;
    int epoll_fd = 0;
    bool need_accept = false;

    epoll_event event;  // TODO remove this field
    ConnectionsList<Connection> connections;
    using ConnectionNode = decltype(connections)::Node_ptr;
    std::vector<ConnectionNode> ready_connections;

    Logger log{"server"};
};
