#pragma once

#include "logger.hpp"

class LinuxConnection
{
public:
    explicit LinuxConnection(int sock);
    virtual ~LinuxConnection() = default;
    LinuxConnection(LinuxConnection&&) = default;
    LinuxConnection& operator=(LinuxConnection&&) = delete;
    LinuxConnection(const LinuxConnection&) = delete;
    LinuxConnection& operator=(const LinuxConnection&) = delete;

    [[nodiscard]] int getSocket() const noexcept;
    [[nodiscard]] bool closed() const noexcept;
    void closeConnection() noexcept;
    virtual std::string readMessage();
    virtual std::string_view writeMessage(const std::string_view message);

private:
    int socket;
    Logger log{"connection"};
};
