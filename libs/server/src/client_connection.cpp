#include "client_connection.hpp"
#include <cassert>

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

[[nodiscard]] bool ClientConnection::requestCompleted() const noexcept
{
    return request_builder.isComplete();
}

HttpRequest ClientConnection::getRequest()
{
    assert(requestCompleted());
    return request_builder.getRequest();
}

void ClientConnection::setResponce(std::string responce)
{
    responce_message = std::move(responce);
}
