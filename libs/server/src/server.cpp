#include "server.hpp"
#include <cassert>

Server::Server(LogicalController&& new_contoller, short port)
: listener(port, *this)
, controller(new_contoller)
{
    poller.addSerialized(&listener);
    logger.log("Start listening. Port: ", listener.port());
}

void Server::addConnection(int socket)
{
    auto new_connection = std::make_shared<ClientConnection>(socket, controller);
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
