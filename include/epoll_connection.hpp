#pragma once

#include "linux_connection.hpp"
#include <cstdint>

class EpollConnection final : public LinuxConnection
{
public:
    using LinuxConnection::LinuxConnection;

    void readRequest() override;
    void solveRequest() override;
    void writeResponse() override;

    std::uint32_t events() const noexcept;
    void setEvents(std::uint32_t new_events) noexcept;

    bool canRead() const noexcept;
    void setCanRead(bool can) noexcept;

    bool canWrite() const noexcept;
    void setCanWrite(bool can) noexcept;

private:
    bool can_read;
    bool can_write;
    std::uint32_t actual_events = 0;
};
