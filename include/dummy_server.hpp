#pragma once

#include "connection.hpp"
#include "listener.hpp"
#include "poller.hpp"
#include "unordered_set"
#include <memory>

class DummyServer : public ConnectionKeeper
{
public:
    DummyServer()
    : listener(8765, *this)
    {
        poller.addSerialized(&listener);
    }

    void addConnection(int socket) override
    {
        auto new_connection = std::make_shared<Connection>(socket);
        auto [iter, inserted] = connections.insert(new_connection);
        assert(inserted);
        poller.addSerialized(iter->get());
        std::cout << "Connection added. Map size: " << connections.size() << std::endl;
    }

    void run()
    {
        for (;;) {
            auto *serialized = poller.wait();
            if (!serialized->wantOut() && !serialized->wantIn()) {
                auto iter = connections.find(serialized);
                assert(iter != connections.end());
                std::cout << "Remove connection" << serialized->getFd() << std::endl;
                connections.erase(iter);
                std::cout << "from map.Map size: " << connections.size() << std::endl;
            }
        }
    }

private:
    struct con_ptr_hash
    {
        using hash_type = std::hash<std::shared_ptr<Connection>>;
        using is_transparent = void;

        std::size_t operator()(const std::shared_ptr<Connection>& con) const
        {
            return hash_type{}(con);
        }

        std::size_t operator()(const void *con) const
        {
            return std::hash<const void *>{}(con);
        }
    };

    struct comparator
    {
        using is_transparent = void;

        bool operator()(const std::shared_ptr<Connection>& lhp, const std::shared_ptr<Connection>& rhp) const
        {
            return lhp == rhp;
        }

        bool operator()(const void *lhp, const std::shared_ptr<Connection>& rhp) const
        {
            return lhp == rhp.get();
        }

        bool operator()(const std::shared_ptr<Connection>& lhp, const void *rhp) const
        {
            return lhp.get() == rhp;
        }
    };

    std::unordered_set<std::shared_ptr<Connection>, con_ptr_hash, comparator> connections;
    Listener listener;
    Poller poller;
};
