#pragma once

#include "client_connection.hpp"
#include "functor_utils.hpp"
#include "listener.hpp"
#include "poller.hpp"
#include <unordered_set>

#include <boost/multi_index/key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index_container.hpp>

class LogicalController;
class FrontendService;

namespace multiindex = boost::multi_index;

class Server : public ConnectionKeeper
{
public:
    Server(std::shared_ptr<LogicalController> controller, std::shared_ptr<FrontendService> frontend = nullptr,
           short port = 8000);
    ~Server() override;
    void addSerialized(AbstractSerialized& serialized);
    void addConnection(int socket) override;
    void run();

private:
    using Connection = ClientConnection;
    using connection_ptr_t = std::shared_ptr<Connection>;

    void eraseConnection(AbstractSerialized *serialized);
    void updateTimeOrder(AbstractSerialized *serialized);

    static const void *connectionAddr(const connection_ptr_t& conn)
    {
        return conn.get();
    }
    static std::chrono::system_clock::time_point connectionWakeup(const connection_ptr_t& conn)
    {
        return conn->getWakeup();
    }

    // clang-format off
    using connections_t = multiindex::multi_index_container<connection_ptr_t,
        multiindex::indexed_by<
            multiindex::hashed_unique<multiindex::global_fun<const connection_ptr_t&, const void *, connectionAddr>>,
            multiindex::ordered_non_unique<
                multiindex::global_fun<const connection_ptr_t&, std::chrono::system_clock::time_point, connectionWakeup>
            >
        >
    >;
    // clang-format on

    Poller poller;
    Listener listener;
    connections_t connections;
    std::shared_ptr<LogicalController> controller;
    std::shared_ptr<FrontendService> frontend;
    Logger logger{"Server"};
};
