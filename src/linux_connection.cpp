#include "linux_connection.hpp"
#include "stdexcept"
#include <cstring>
#include <sstream>
#include <unistd.h>

LinuxConnection::LinuxConnection(int sock)
: socket(sock)
{}

int LinuxConnection::getSocket() const noexcept
{
    return socket;
}

bool LinuxConnection::closed() const noexcept
{
    return socket == -1;
}

void LinuxConnection::closeConnection() noexcept
{
    log.log("close connection");
    close(socket);
    socket = -1;
}

void LinuxConnection::readRequest()
{
    constexpr int buff_size = 256;
    char buff[buff_size];
    std::stringstream body;
    int read_count;
    do {
        read_count = read(socket, buff, buff_size);
        if (read_count == -1) {
            if (errno == EAGAIN) {
                log.log("eagain");
                break;
            }
            throw std::runtime_error(std::string("read() error: ") +
                                     std::strerror(errno));
        }
        body.write(buff, read_count);
        log.log("read_count:", read_count);
        log.log("buffer read:", buff, read_count);
    } while (read_count > 0);
    if (read_count == 0) {
        closeConnection();
    }
    actual_request.complete(body.str());
    log.log("create request");
}

void LinuxConnection::solveRequest()
{
    response.complete(actual_request.getBody());
    log.log("request solved");
    actual_request.reset();
}

void LinuxConnection::writeResponse()
{
    const std::string& body = response.getBody();
    log.log("start write body", body);
    int writed = write(socket, body.c_str(), body.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") +
                                 std::strerror(errno));
    }
    log.log("write done");
    response.reset();
}

bool LinuxConnection::writeReady() const noexcept
{
    return response.isComplete();
}
