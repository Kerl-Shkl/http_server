#pragma once

#include "linux_connection.hpp"
#include <cstdint>

class EpollConnection : public LinuxConnection
{
public:
    using LinuxConnection::LinuxConnection;

    std::string readMessage() override;
    std::string_view writeMessage(const std::string_view message) override;

    [[nodiscard]] std::uint32_t events() const noexcept;
    void setEvents(std::uint32_t new_events) noexcept;

    [[nodiscard]] bool canRead() const noexcept;
    void setCanRead(bool can) noexcept;

    [[nodiscard]] bool canWrite() const noexcept;
    void setCanWrite(bool can) noexcept;

private:
    bool can_read;
    bool can_write;
    std::uint32_t actual_events = 0;
};
