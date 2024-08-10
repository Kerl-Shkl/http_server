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
    requests.push(Request{body.str()});
    log.log("create request");
}

void Connection::solveRequest()
{
    if (requests.empty()) {
        log.log("request queue is empty");
        return;
    }
    log.log("start solve request");
    Request request = std::move(requests.front());
    requests.pop();
    const std::string& body = request.getBody();
    log.log("start write body", body);
    int writed = write(socket, body.c_str(), body.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") +
                                 std::strerror(errno));
    }
    log.log("write done");
}

bool Connection::writeReady() const noexcept
{
    return !requests.empty();
}
