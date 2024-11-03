#include "linux_connection.hpp"
#include "stdexcept"
#include <array>
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

std::string LinuxConnection::readMessage()
{
    constexpr int buff_size = 256;
    std::array<char, buff_size> buff{};
    std::stringstream body;
    ssize_t read_count{};
    do {
        read_count = read(socket, buff.data(), buff.size());
        if (read_count == -1) {
            if (errno == EAGAIN) {
                log.log("eagain");
                break;
            }
            throw std::runtime_error(std::string("read() error: ") + std::strerror(errno));
        }
        body.write(buff.data(), read_count);
        log.log("read_count:", read_count);
        log.log("buffer read:", buff.data(), static_cast<int>(read_count));
    } while (read_count > 0);
    if (read_count == 0) {
        closeConnection();
    }
    std::string result = body.str();
    log.log("read new message: ", result);
    return result;
}

std::string_view LinuxConnection::writeMessage(const std::string_view message)
{
    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
    int writed = write(socket, message.data(), message.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") + std::strerror(errno));
    }
    std::string_view left = message.substr(writed);
    log.log("write message", message.substr(0, writed));
    return left;
}
