#pragma once

#include "epoll_connection.hpp"
#include "request_builder.hpp"

class ClientConnection final : private EpollConnection
{
public:
    using EpollConnection::EpollConnection;

    void readNewMessage();
    [[nodiscard]] bool writeReady() const noexcept;
    void writeMessage();
    [[nodiscard]] bool requestCompleted() const noexcept;
    HttpRequest extractRequest();
    void setResponce(std::string responce);

    using EpollConnection::canRead;
    using EpollConnection::canWrite;
    using EpollConnection::events;
    using EpollConnection::setCanRead;
    using EpollConnection::setCanWrite;
    using EpollConnection::setEvents;
    using LinuxConnection::closeConnection;
    using LinuxConnection::closed;
    using LinuxConnection::getSocket;

private:
    RequestBuilder request_builder;
    std::string responce_message;
};
