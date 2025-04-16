#include "linux_connection.hpp"
#include "stdexcept"
#include <array>
#include <cassert>
#include <cstring>
#include <sstream>
#include <unistd.h>

LinuxConnection::LinuxConnection()
: log("LinuxConnection: ?")
{}

LinuxConnection::LinuxConnection(int sock)
: socket(sock)
, log("LinuxConnection:" + std::to_string(socket))
{}

void LinuxConnection::setSocket(int s) noexcept
{
    assert(socket == -1);
    socket = s;
}

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
                break;
            }
            throw std::runtime_error(std::string("read() error: ") + std::strerror(errno));
        }
        body.write(buff.data(), read_count);
    } while (read_count > 0);
    auto result = body.str();
    log.log("read:", result, true);
    if (read_count == 0) {
        closeConnection();
    }
    return result;
}

std::vector<uint8_t> LinuxConnection::readBuffer()
{
    constexpr int batch_size = 256;
    size_t fullness = 0;
    std::vector<uint8_t> buff(batch_size, 0);
    ssize_t read_count{};
    do {
        assert(fullness < buff.size());
        read_count = read(socket, buff.data() + fullness, buff.size() - fullness);
        if (read_count == -1) {
            if (errno == EAGAIN) {
                break;
            }
            throw std::runtime_error(std::string("read() error: ") + std::strerror(errno));
        }
        fullness += read_count;
        buff.resize(fullness + batch_size);
    } while (read_count > 0);
    buff.resize(fullness);
    log.log("read buffer. buffer_size: ", buff.size());
    if (read_count == 0) {
        closeConnection();
    }
    return buff;
}

std::string_view LinuxConnection::writeMessage(const std::string_view message)
{
    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
    int writed = write(socket, message.data(), message.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") + std::strerror(errno));
    }
    return message.substr(writed);
}

int LinuxConnection::writeBuffer(const std::span<uint8_t> buffer)
{
    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
    int writed = write(socket, buffer.data(), buffer.size());
    if (writed == -1) {
        throw std::runtime_error(std::string("write() error: ") + std::strerror(errno));
    }
    return writed;
}
