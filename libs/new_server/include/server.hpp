#pragma once

#include "client_connection.hpp"
#include "listener.hpp"
#include "logical_controller.hpp"
#include "poller.hpp"
#include "ptr_utils.hpp"
#include <unordered_set>

class Server : public ConnectionKeeper
{
public:
    Server(LogicalController&& controller, short port = 8000);
    void addConnection(int socket) override;
    void run();

private:
    using Connection = ClientConnection;
    using connections_t =
        std::unordered_set<std::shared_ptr<Connection>, PtrHash<Connection>, PtrEqualTo<Connection>>;

    Poller poller;
    Listener listener;
    connections_t connections;
    LogicalController controller;
    Logger logger{"Server"};
};
