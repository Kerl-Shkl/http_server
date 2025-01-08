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
