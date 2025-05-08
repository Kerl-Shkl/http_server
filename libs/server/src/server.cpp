#include "server.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"
#include <cassert>

Server::Server(std::shared_ptr<LogicalController> control, std::shared_ptr<FrontendService> front, short port)
: listener(port, *this)
, controller(std::move(control))
{
    poller.addSerialized(&listener);
    if (front != nullptr) {
        poller.addSerialized(&front->getResourceObserver());
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
    auto [iter, inserted] = connections.emplace(
        ConnectionNode{.connection = std::move(new_connection), .timeout = nextTimeout()});
    assert(inserted);
    poller.addSerialized(iter->connection.get());
    logger.log("Connection added. Connections count: ", connections.size());
}

void Server::run()
{
    for (;;) {
        auto *serialized = poller.wait(getWaitingTimeout());
        if (serialized == nullptr) {
            clearTimeoutedConnections();
            continue;
        }
        if (!serialized->wantOut() && !serialized->wantIn()) {
            eraseConnection(serialized);
        }
        else {
            updateTimeOrder(serialized);
        }
    }
}

void Server::clearTimeoutedConnections()
{
    auto& index = connections.get<1>();
    auto now = std::chrono::system_clock::now();
    for (auto it = index.begin(); it != index.end();) {
        if (it->timeout > now) {
            break;
        }
        it->connection->closeConnection();
        poller.removeSerialized(it->connection.get());  // remove from poller
        it = index.erase(it);
    }
}

void Server::eraseConnection(AbstractSerialized *serialized)
{
    auto& index = connections.get<0>();
    if (auto iter = index.find(serialized); iter != index.end()) {
        index.erase(serialized);
        logger.log("Remove connection. Connections count: ", connections.size());
    }
}

void Server::updateTimeOrder(AbstractSerialized *serialized)
{
    auto& index = connections.get<0>();
    if (auto iter = index.find(serialized); iter != index.end()) {
        auto node = index.extract(iter);
        node.value().timeout = nextTimeout();
        index.insert(std::move(node));
    }
}

[[nodiscard]] Server::time_point_t Server::nextTimeout() const noexcept
{
    return std::chrono::system_clock::now() + hold_connection_time;
}

int Server::getWaitingTimeout() const
{
    namespace chrono = std::chrono;
    if (connections.empty()) {
        return -1;
    }
    const auto& index = connections.get<1>();
    time_point_t timeout = index.begin()->timeout;
    auto milisecs = chrono::duration_cast<chrono::milliseconds>(timeout - chrono::system_clock::now());
    return (milisecs > chrono::milliseconds::zero()) ? milisecs.count() : 0;
}
