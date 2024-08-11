#include "epoll_connection.hpp"

void EpollConnection::readRequest()
{
    LinuxConnection::readRequest();
    can_read = false;
}

void EpollConnection::solveRequest()
{
    LinuxConnection::solveRequest();
}

void EpollConnection::writeResponse()
{
    LinuxConnection::writeResponse();
    can_write = false;
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
