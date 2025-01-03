#include "server.hpp"
#include "logical_controller.hpp"
#include <cassert>

Server::Server(std::shared_ptr<LogicalController> new_controller, short port)
: listener(port, *this)
, controller(std::move(new_controller))
{
    poller.addSerialized(&listener);
    logger.log("Start listening. Port: ", listener.port());
}

Server::~Server() = default;

void Server::addConnection(int socket)
{
    auto new_connection = std::make_shared<ClientConnection>(socket, *controller);
    auto [iter, inserted] = connections.insert(new_connection);
    assert(inserted);
    poller.addSerialized(iter->get());
    logger.log("Connection added. Connections count: ", connections.size());
}

void Server::run()
{
    for (;;) {
        auto *serialized = poller.wait();

        if (!serialized->wantOut() && !serialized->wantIn()) {
            auto iter = connections.find(serialized);
            assert(iter != connections.end());
            connections.erase(iter);
            logger.log("Remove connection. Connections count: ", connections.size());
        }
    }
}
