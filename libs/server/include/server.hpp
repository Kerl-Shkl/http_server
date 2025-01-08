#pragma once

#include "client_connection.hpp"
#include "functor_utils.hpp"
#include "listener.hpp"
#include "poller.hpp"
#include <unordered_set>

class LogicalController;
class FrontendService;

class Server : public ConnectionKeeper
{
public:
    Server(std::shared_ptr<LogicalController> controller, std::shared_ptr<FrontendService> frontend = nullptr,
           short port = 8000);
    ~Server() override;
    void addConnection(int socket) override;
    void run();

private:
    using Connection = ClientConnection;
    using connections_t =
        std::unordered_set<std::shared_ptr<Connection>, PtrHash<Connection>, PtrEqualTo<Connection>>;

    Poller poller;
    Listener listener;
    connections_t connections;
    std::shared_ptr<LogicalController> controller;
    std::shared_ptr<FrontendService> frontend;
    Logger logger{"Server"};
};
