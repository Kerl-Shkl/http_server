#pragma once

#include "client_connection.hpp"
#include "connections_list.hpp"
#include "logger.hpp"
#include "logical_controller.hpp"
#include <sys/epoll.h>
#include <vector>

class Server
{
public:
    Server(LogicalController&& controller);
    void run();

private:
    using Connection = ClientConnection;

    void waitConnection();
    [[nodiscard]] bool newClients() const noexcept;
    void returnConnectionToWaiters(Connection& connection);
    void doIO(Connection& connection);

    void createListenSocket();
    void addListenToEpoll();
    int acceptConnection();
    void addConnection();
    void createEpoll();

    static constexpr short port = 8000;  // NOLINT
    int listen_sd = 0;
    int epoll_fd = 0;
    bool need_accept = false;

    ConnectionsList<Connection> connections;
    using ConnectionNode = decltype(connections)::Node_ptr;
    std::vector<ConnectionNode> ready_connections;
    LogicalController logical_controller;

    Logger log{"server"};
};
