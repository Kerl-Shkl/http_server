#include "server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
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
        waitConnection();
        if (newClients()) {
            addConnection();
        }
        for (auto& connection_node : ready_connections) {
            Connection& connection = connections.getConnection(connection_node);
            doIO(connection);

            if (connection.closed()) {
                log.log("connection closed");
                connections.removeNode(connection_node);
            }
            else {
                returnConnectionToWaiters(connection);
            }
        }
        ready_connections.clear();
    }
}

void Server::waitConnection()
{
    int waited_count{};
    epoll_event event{};  // TODO add more then 1 event in epoll result
    while ((waited_count = epoll_wait(epoll_fd, &event, 1, -1)) < 1) {
        if (errno != EINTR) {
            throw std::runtime_error(std::string("epoll_wait() error: ") + std::strerror(errno));
        }
    }
    if (event.data.fd == listen_sd) {
        log.log("listen socket waited");
        need_accept = true;
    }
    else {
        ConnectionNode connection_node = static_cast<ConnectionNode>(event.data.ptr);
        Connection& connection = connections.getConnection(connection_node);
        connection.setCanRead(static_cast<bool>(event.events & EPOLLIN));
        connection.setCanWrite(static_cast<bool>(event.events & EPOLLOUT));
        ready_connections.emplace_back(connection_node);
    }
}

bool Server::newClients() const noexcept
{
    return need_accept;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Server::returnConnectionToWaiters(Connection& connection)
{
    std::uint32_t new_events = connection.writeReady() ? EPOLLIN | EPOLLOUT : EPOLLIN;
    if (new_events != connection.events()) {
        connection.setEvents(new_events);
        epoll_event event = {.events = new_events, .data = {.ptr = &connection}};
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connection.getSocket(), &event);
    }
}

void Server::doIO(Connection& connection)
{
    if (connection.canRead()) {
        connection.readRequest();
        connection.solveRequest();
    }
    if (connection.canWrite()) {
        connection.writeResponse();
    }
}

int Server::acceptConnection()
{
    sockaddr_in addr{};
    socklen_t addrlen = sizeof(addr);
    int socket = accept(listen_sd, reinterpret_cast<sockaddr *>(&addr), &addrlen);
    if (socket == -1) {
        if (errno == EAGAIN) {
            return 0;
        }
        throw std::runtime_error(std::string("accept() error: ") + std::strerror(errno));
    }
    log.log("connection accepted");

    int flags = fcntl(socket, F_GETFL);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    return socket;
}

void Server::addConnection()
{
    int socket = 0;
    while ((socket = acceptConnection()) != 0) {
        connections.pushBack(Connection{socket});
        epoll_event new_event = {.events = EPOLLIN, .data = {.ptr = connections.getBackNode()}};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &new_event);
    }
    need_accept = false;
}

void Server::createListenSocket()
{
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd == -1) {
        throw std::runtime_error(std::string("socket() error: ") + std::strerror(errno));
    }
    int opt = 1;
    setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    int err = bind(listen_sd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (err == -1) {
        throw std::runtime_error(std::string("bind() error: ") + std::strerror(errno));
    }
    err = listen(listen_sd, SOMAXCONN);
    if (err == -1) {
        throw std::runtime_error(std::string("listen() error: ") + std::strerror(errno));
    }
    int flags = fcntl(listen_sd, F_GETFL);
    fcntl(listen_sd, F_SETFL, flags | O_NONBLOCK);
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
        throw std::runtime_error(std::string("epoll_create1() error: ") + std::strerror(errno));
    }
}
