#pragma once

#include "backend_interface.hpp"
#include "client_connection.hpp"
#include "listener.hpp"
#include "poller.hpp"

#include <boost/multi_index/key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index_container.hpp>

class LogicalController;
class FrontendService;

namespace multiindex = boost::multi_index;

class Server : public ConnectionKeeper
{
public:
    Server(std::string resource_dir, short port = 8000);
    ~Server() override;
    void setBackendService(std::shared_ptr<BackendInterface> backend_service);
    void addSerialized(AbstractSerialized& serialized);
    void addConnection(int socket) override;  // Change may be (add with ClientConnection not socket fd)
    void run();

private:
    using connection_ptr_t = std::shared_ptr<ClientConnection>;
    using time_point_t = std::chrono::system_clock::time_point;
    struct ConnectionNode
    {
        connection_ptr_t connection;
        time_point_t timeout;
    };

    void eraseConnection(AbstractSerialized *serialized);
    void updateTimeOrder(AbstractSerialized *serialized);
    [[nodiscard]] int getWaitingTimeout() const;
    [[nodiscard]] time_point_t nextTimeout() const noexcept;
    void clearTimeoutedConnections();

    static const void *connectionAddr(const ConnectionNode& node)
    {
        return node.connection.get();
    }

    // clang-format off
    using connections_t = multiindex::multi_index_container<ConnectionNode,
        multiindex::indexed_by<
            multiindex::hashed_unique<multiindex::global_fun<const ConnectionNode&, const void *, connectionAddr>>,
            multiindex::ordered_non_unique<multiindex::member<ConnectionNode, time_point_t, &ConnectionNode::timeout>
            >
        >
    >;
    // clang-format on

    Poller poller;
    Listener listener;
    connections_t connections;
    static constexpr auto hold_connection_time = std::chrono::minutes(2);
    std::shared_ptr<LogicalController> controller;
    std::shared_ptr<FrontendService> frontend;
    std::shared_ptr<BackendInterface> backend{nullptr};
    Logger logger{"Server"};
};
