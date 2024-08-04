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
    char buff[256];
    std::stringstream body;
    int read_count;
    do {
        read_count = read(socket, buff, buff_size);
        if (read_count == -1) {
            throw std::runtime_error(std::string("read() error:") +
                                     std::strerror(errno));
        }
        body.write(buff, read_count);
        std::cout << "[connection] read_count: " << read_count
                  << " buffer read:";
        std::cout.write(buff, read_count);
        std::cout << std::endl;
    } while (read_count > 0);

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
    std::cout << "[connection] start write body: " << body << std::endl;
    write(socket, body.c_str(), body.size());
    std::cout << "[connection] write done" << std::endl;
}
