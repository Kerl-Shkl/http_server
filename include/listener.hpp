#pragma once

#include "serialized_interface.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

class ConnectionKeeper
{
public:
    virtual ~ConnectionKeeper() = default;
    virtual void addConnection(int socket) = 0;
};

class Listener : public AbstractSerialized
{
public:
    Listener(int port, ConnectionKeeper& keeper)
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
        std::cout << "listen socker created" << std::endl;
    }

    void handleIn() override
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
        std::cout << "connection accepted" << std::endl;
        int flags = fcntl(socket, F_GETFL);
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
        conkeeper.addConnection(socket);
    }

    void handleOut() override
    {
        std::cout << "Something went wrong. Out from listen socket???" << std::endl;
        assert(false);
    }

    [[nodiscard]] int getFd() const override
    {
        return fd;
    }
    [[nodiscard]] bool wantIn() const override
    {
        return true;
    }
    [[nodiscard]] bool wantOut() const override
    {
        return false;
    }

private:
    int fd{};
    int listen_port;
    ConnectionKeeper& conkeeper;
};
