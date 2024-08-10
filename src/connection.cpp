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
                std::cout << "[connection] eagain" << std::endl;
                break;
            }
            else {
                throw std::runtime_error(std::string("read() error: ") +
                                         std::strerror(errno));
            }
        }
        body.write(buff, read_count);
        std::cout << "[connection] read_count: " << read_count
                  << " buffer read:";
        std::cout.write(buff, read_count);
        if (buff[read_count - 1] != '\n') {
            std::cout << std::endl;
        }
    } while (read_count > 0);
    if (read_count == 0) {
        close(socket);
    }

    requests.push(Request{body.str()});
    std::cout << "[connection] create request" << std::endl;
}

void Connection::solveRequest()
{
    if (requests.empty()) {
        std::cout << "[connection] request queue is empty" << std::endl;
        return;
    }
    std::cout << "[connection] start solve request" << std::endl;
    Request request = std::move(requests.front());
    requests.pop();
    const std::string& body = request.getBody();
    std::cout << "[connection] start write body: " << body;
    if (body.back() != '\n') {
        std::cout << std::endl;
    }
    int writed = write(socket, body.c_str(), body.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") +
                                 std::strerror(errno));
    }
    std::cout << "[connection] write done" << std::endl;
}

bool Connection::writeReady() const noexcept
{
    return !requests.empty();
}
