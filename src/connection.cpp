#include "connection.hpp"
#include "stdexcept"
#include <cstring>
#include <iostream>
#include <sstream>
#include <unistd.h>

Connection::Connection(int sock)
: socket(sock)
{}

void Connection::readRequest()
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
            else {
                throw std::runtime_error(std::string("read() error: ") +
                                         std::strerror(errno));
            }
        }
        body.write(buff, read_count);
        log.log("read_count:", read_count);
        log.log("buffer read:", buff, read_count);
    } while (read_count > 0);
    if (read_count == 0) {
        close(socket);
    }
    actual_request.complete(body.str());
    log.log("create request");
}

void Connection::solveRequest()
{
    response.complete(actual_request.getBody());
    log.log("request solved");
    actual_request.reset();
}

void Connection::writeRequest()
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

bool Connection::writeReady() const noexcept
{
    return response.isComplete();
}
