#include "poller.hpp"
#include "abstract_serialized.hpp"
#include <cassert>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

Poller::Poller()
{
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
        throw std::runtime_error(std::string("epoll_create1() error: ") + std::strerror(errno));
    }
}

Poller::~Poller()
{
    close(epoll_fd);
    epoll_fd = -1;
}

void Poller::addSerialized(AbstractSerialized *serialized)
{
    assert(serialized->actual_events == 0);
    uint32_t events = (serialized->wantIn() ? EPOLLIN : 0) | (serialized->wantOut() ? EPOLLOUT : 0);
    assert(events != 0);
    epoll_event new_event = {.events = events, .data = {.ptr = serialized}};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serialized->getFd(), &new_event);
    serialized->poller = this;
}

AbstractSerialized *Poller::wait()
{
    int waited_count{};
    epoll_event event{};  // TODO add more then 1 event in epoll result
    while ((waited_count = epoll_wait(epoll_fd, &event, 1, -1)) < 1) {
        if (errno != EINTR) {
            throw std::runtime_error(std::string("epoll_wait() error: ") + std::strerror(errno));
        }
    }
    auto *serialized = static_cast<AbstractSerialized *>(event.data.ptr);
    handleIO(*serialized, event.events);
    updateSerializedMode(*serialized);
    return serialized;
}

AbstractSerialized *Poller::check()
{
    epoll_event event{};  // TODO add more then 1 event in epoll result
    int waited_count = epoll_wait(epoll_fd, &event, 1, 0);
    if (waited_count < 0) {
        throw std::runtime_error(std::string("epoll_wait() error: ") + std::strerror(errno));
    }
    if (waited_count != 0) {
        auto *serialized = static_cast<AbstractSerialized *>(event.data.ptr);
        handleIO(*serialized, event.events);
        updateSerializedMode(*serialized);
        return serialized;
    }
    return nullptr;
}

void Poller::handleIO(AbstractSerialized& serialized, uint32_t events) const
{
    if (events & EPOLLIN) {
        serialized.handleIn();
    }
    if (events & EPOLLOUT) {
        serialized.handleOut();
    }
}

void Poller::updateSerializedMode(AbstractSerialized& serialized) const
{
    uint32_t new_waiting = (serialized.wantIn() ? EPOLLIN : 0) | (serialized.wantOut() ? EPOLLOUT : 0);
    epoll_event new_event = {.events = new_waiting, .data = {.ptr = &serialized}};
    if (new_waiting == 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, serialized.getFd(), &new_event);
    }
    else if (serialized.actual_events != new_waiting) {
        serialized.actual_events = new_waiting;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, serialized.getFd(), &new_event);
    }
}
