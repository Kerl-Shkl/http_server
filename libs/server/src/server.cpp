#include "server.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"
#include <cassert>

Server::Server(std::shared_ptr<LogicalController> control, std::shared_ptr<FrontendService> front, short port)
: listener(port, *this)
, controller(std::move(control))
, frontend(std::move(front))
{
    poller.addSerialized(&listener);
    if (frontend != nullptr) {
        poller.addSerialized(&frontend->getResourceObserver());
    }
    logger.log("Start listening. Port: ", listener.port());
}

Server::~Server() = default;

void Server::addSerialized(AbstractSerialized& serialized)
{
    poller.addSerialized(&serialized);
}

void Server::addConnection(int socket)
{
    auto new_connection = std::make_shared<ClientConnection>(socket, *controller);
    new_connection->updateWakeup();
    auto [iter, inserted] = connections.insert(new_connection);
    assert(inserted);
    poller.addSerialized(iter->get());
    logger.log("Connection added. Connections count: ", connections.size());
}

void Server::run()
{
    for (;;) {
        auto *serialized = poller.wait(-1);

        if (!serialized->wantOut() && !serialized->wantIn()) {
            eraseConnection(serialized);
            logger.log("Remove connection. Connections count: ", connections.size());
        }
        else {
            updateTimeOrder(serialized);
        }
    }
}

void Server::eraseConnection(AbstractSerialized *serialized)
{
    auto& index = connections.get<0>();
    index.erase(serialized);
}

void Server::updateTimeOrder(AbstractSerialized *serialized)
{
    auto& index = connections.get<0>();
    auto iter = index.find(serialized);
    assert(iter != index.end());
    auto node = index.extract(iter);
    node.value()->updateWakeup();
    index.insert(std::move(node));
}
