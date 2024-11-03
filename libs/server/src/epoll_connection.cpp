#include "epoll_connection.hpp"

std::string EpollConnection::readMessage()
{
    can_read = false;
    return LinuxConnection::readMessage();
}

std::string_view EpollConnection::writeMessage(const std::string_view message)
{
    can_write = false;
    return LinuxConnection::writeMessage(message);
}

bool EpollConnection::canRead() const noexcept
{
    return can_read;
}
bool EpollConnection::canWrite() const noexcept
{
    return can_write;
}

void EpollConnection::setCanRead(bool can) noexcept
{
    can_read = can;
}

void EpollConnection::setCanWrite(bool can) noexcept
{
    can_write = can;
}

std::uint32_t EpollConnection::events() const noexcept
{
    return actual_events;
}

void EpollConnection::setEvents(std::uint32_t new_events) noexcept
{
    actual_events = new_events;
}
