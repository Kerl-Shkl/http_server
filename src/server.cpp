#include "server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

Server::Server()
{
    createListenSocket();
}

void Server::run()
{
    Connection connection = acceptConnection();
    connection.readRequest();
    connection.solveRequest();
    acceptConnection();
}

Connection Server::acceptConnection()
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int socket =
        accept(listen_sd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
    if (socket == -1) {
        throw std::runtime_error(std::string("accept() error:") +
                                 std::strerror(errno));
    }
    std::cout << "[server] connection accepted" << std::endl;

    // int flags = fcntl(socket, F_GETFL);
    // fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    return Connection{socket};
}

void Server::createListenSocket()
{
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd == -1) {
        throw std::runtime_error(std::string("socket() error:") +
                                 std::strerror(errno));
    }

    int opt = 1;
    setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    int err = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (err == -1) {
        throw std::runtime_error(std::string("bind() error:") +
                                 std::strerror(errno));
    }
    err = listen(listen_sd, 16);
    if (err == -1) {
        throw std::runtime_error(std::string("listen() error:") +
                                 std::strerror(errno));
    }
    std::cout << "[server] listen socket created" << std::endl;
}
