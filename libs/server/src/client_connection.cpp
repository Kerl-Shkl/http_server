#include "client_connection.hpp"
#include <cassert>

ClientConnection::ClientConnection(int socket, LogicalController& new_controller)
: lcon(socket)
, controller(new_controller)
{}

void ClientConnection::handleIn()
{
    try {
        std::string message = lcon.readMessage();
        request_builder.complete(message);
        processRequest();
    }
    catch (const std::exception& exception) {
        logger.log("handleIn error: ", exception.what());
        lcon.closeConnection();
    }
}

void ClientConnection::handleOut()
{
    try {
        if (!closed()) {
            auto left = lcon.writeMessage(response_message);
            response_message = std::string{left};
        }
    }
    catch (const std::exception& exception) {
        logger.log("handleOut error: ", exception.what());
        lcon.closeConnection();
    }
}

[[nodiscard]] int ClientConnection::getFd() const
{
    return lcon.getSocket();
}

[[nodiscard]] bool ClientConnection::wantIn() const
{
    // May be return responce_message.empty() && !request_builder.isComplete()
    return !closed();
}

void ClientConnection::processRequest()
{
    if (!request_builder.isComplete()) {
        return;
    }
    auto request = extractRequest();
    auto response = controller.process(request);
    response_message = response.buildMessage();
}

[[nodiscard]] bool ClientConnection::wantOut() const
{
    return !closed() && !response_message.empty();
}

void ClientConnection::closeConnection()
{
    lcon.closeConnection();
}

[[nodiscard]] bool ClientConnection::closed() const
{
    return lcon.closed();
}

HttpRequest ClientConnection::extractRequest()
{
    assert(request_builder.isComplete());
    auto request = request_builder.getRequest();
    request_builder.reset();
    return request;
}
