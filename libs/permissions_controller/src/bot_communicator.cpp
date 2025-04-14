#include "bot_communicator.hpp"
#include "permissions_controller.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

BotCommunicator::BotCommunicator(PermissionsController& controller)
: permissions_controller{controller}
{
    int socket_fd = openSocket();
    lcon.setSocket(socket_fd);
}

int BotCommunicator::openSocket() const
{
    sockaddr_un serv_addr{};
    memset(reinterpret_cast<uint8_t *>(&serv_addr), 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "/tmp/my_socket");
    int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error{"Can't create unix socket for tg bot"};
    }
    int conn_res = connect(sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr), servlen);
    if (conn_res < 0) {
        throw std::runtime_error{"Can't connect unix socket for tg bot"};
    }
    return sockfd;
}

void BotCommunicator::handleIn()
{
    try {
        std::string message = lcon.readMessage();
        // TODO convert message to response
    }
    catch (const std::exception& exception) {
        logger.log("handleIn error: ", exception.what());
        lcon.closeConnection();
    }
}

void BotCommunicator::handleOut()
{
    try {
        if (!lcon.closed()) {
            // TODO add write another request
            // auto left = lcon.writeMessage();
            // response_message = std::string{left};
        }
    }
    catch (const std::exception& exception) {
        logger.log("handleOut error: ", exception.what());
        lcon.closeConnection();
    }
}

[[nodiscard]] int BotCommunicator::getFd() const
{
    return lcon.getSocket();
}
[[nodiscard]] bool BotCommunicator::wantIn() const
{
    return !lcon.closed();
}
[[nodiscard]] bool BotCommunicator::wantOut() const
{
    return !lcon.closed() /* && there is something to write */;
}
