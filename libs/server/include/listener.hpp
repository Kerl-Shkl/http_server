#pragma once

#include "abstract_serialized.hpp"
#include "logger.hpp"

class ConnectionKeeper
{
public:
    virtual ~ConnectionKeeper() = default;
    virtual void addConnection(int socket) = 0;
};

class Listener : public AbstractSerialized
{
public:
    Listener(short port, ConnectionKeeper& keeper);

    void handleIn() override;
    void handleOut() override;

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;

    [[nodiscard]] short port() const noexcept;

private:
    int fd = -1;
    short listen_port;
    ConnectionKeeper& conkeeper;
    Logger logger{"listener"};
};
