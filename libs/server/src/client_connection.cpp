#include "client_connection.hpp"

void ClientConnection::readNewMessage()
{
    std::string message = EpollConnection::readMessage();
    request_builder.complete(message);
}

[[nodiscard]] bool ClientConnection::writeReady() const noexcept
{
    return !responce_message.empty();
}

void ClientConnection::writeMessage()
{
    auto left = EpollConnection::writeMessage(responce_message);
    responce_message = std::string{left};
}
