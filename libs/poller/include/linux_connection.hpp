#pragma once

#include "logger.hpp"
#include <cstdint>
#include <span>
#include <vector>

class LinuxConnection
{
public:
    LinuxConnection();
    explicit LinuxConnection(int sock);
    ~LinuxConnection() = default;
    LinuxConnection(LinuxConnection&&) = default;
    LinuxConnection& operator=(LinuxConnection&&) = delete;
    LinuxConnection(const LinuxConnection&) = delete;
    LinuxConnection& operator=(const LinuxConnection&) = delete;

    void setSocket(int s) noexcept;
    [[nodiscard]] int getSocket() const noexcept;
    [[nodiscard]] bool closed() const noexcept;
    void closeConnection() noexcept;
    std::string readMessage();
    std::vector<uint8_t> readBuffer();
    std::string_view writeMessage(const std::string_view message);
    int writeBuffer(const std::span<uint8_t> buffer);

private:
    int socket = -1;
    Logger log;
};
