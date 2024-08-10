#include "server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

Server::Server()
{
    createEpoll();
    createListenSocket();
    addListenToEpoll();
}

void Server::run()
{
    for (;;) {
        log.log("start epoll wait");
        if (epoll_wait(epoll_fd, &event, 1, -1) < 1) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("epoll_wait() error: ") +
                                     std::strerror(errno));
        }
        if (event.data.fd == listen_sd) {
            log.log("listen socket waited");
            addConnection();
        }
        else {
            log.log("io socket waited");
            auto it = connections.find(event.data.fd);
            Connection& connection = it->second;
            if (event.events & EPOLLIN) {
                log.log("io socket read ready");
                connection.readRequest();
            }
            if (event.events & EPOLLOUT) {
                log.log("io socket write ready");
                connection.solveRequest();
            }
            if (connection.writeReady()) {
                epoll_event event_with_write{.events = EPOLLOUT | EPOLLIN,
                                             .data = {.fd = event.data.fd}};
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_with_write.data.fd,
                          &event_with_write);
            }
            else {
                epoll_event event_with_write{.events = EPOLLIN,
                                             .data = {.fd = event.data.fd}};
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_with_write.data.fd,
                          &event_with_write);
            }
        }
    }
}

int Server::acceptConnection()
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int socket =
        accept(listen_sd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
    if (socket == -1) {
        throw std::runtime_error(std::string("accept() error: ") +
                                 std::strerror(errno));
    }
    log.log("connection accepted");

    int flags = fcntl(socket, F_GETFL);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    return socket;
}

void Server::addConnection()
{
    int socket = acceptConnection();
    epoll_event new_event = {.events = EPOLLIN, .data = {.fd = socket}};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &new_event);
    Connection connection{socket};
    connections.insert({socket, Connection{connection}});
}

void Server::createListenSocket()
{
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd == -1) {
        throw std::runtime_error(std::string("socket() error: ") +
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
        throw std::runtime_error(std::string("bind() error: ") +
                                 std::strerror(errno));
    }
    err = listen(listen_sd, SOMAXCONN);
    if (err == -1) {
        throw std::runtime_error(std::string("listen() error: ") +
                                 std::strerror(errno));
    }
    log.log("listen socket created");
}

void Server::addListenToEpoll()
{
    epoll_event listen_event = {.events = EPOLLIN, .data = {.fd = listen_sd}};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sd, &listen_event);
}

void Server::createEpoll()
{
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
        throw std::runtime_error(std::string("epoll_create1() error: ") +
                                 std::strerror(errno));
    }
}
