#include "listener.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

Listener::Listener(short port, ConnectionKeeper& keeper)
: listen_port(port)
, conkeeper(keeper)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw std::runtime_error(std::string("socket() error: ") + std::strerror(errno));
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(listen_port);
    int err = bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (err == -1) {
        throw std::runtime_error(std::string("bind() error: ") + std::strerror(errno));
    }
    err = listen(fd, SOMAXCONN);
    if (err == -1) {
        throw std::runtime_error(std::string("listen() error: ") + std::strerror(errno));
    }
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Listener::handleIn()
{
    sockaddr_in addr{};
    socklen_t addrlen = sizeof(addr);
    int socket = accept(fd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
    if (socket == -1) {
        if (errno == EAGAIN) {
            return;
        }
        throw std::runtime_error(std::string("accept() error: ") + std::strerror(errno));
    }
    int flags = fcntl(socket, F_GETFL);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    logger.log("Accept new connection. fd: ", socket);
    conkeeper.addConnection(socket);
}

void Listener::handleOut()
{
    logger.log("Something went wrong. Listener out??");
    assert(false);
}

[[nodiscard]] int Listener::getFd() const
{
    return fd;
}
[[nodiscard]] bool Listener::wantIn() const
{
    return true;
}
[[nodiscard]] bool Listener::wantOut() const
{
    return false;
}

[[nodiscard]] short Listener::port() const noexcept
{
    return listen_port;
}
