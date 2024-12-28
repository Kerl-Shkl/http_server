#pragma once

#include "serialized_interface.hpp"
#include <cassert>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>

class Poller
{
public:
    Poller()
    {
        epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd < 0) {
            throw std::runtime_error(std::string("epoll_create1() error: ") + std::strerror(errno));
        }
    }

    void addSerialized(AbstractSerialized *serialized)
    {
        std::cout << "Add serialized to epoll" << std::endl;
        assert(serialized->actual_events == 0);
        uint32_t events = (serialized->wantIn() ? EPOLLIN : 0) | (serialized->wantOut() ? EPOLLOUT : 0);
        assert(events != 0);
        epoll_event new_event = {.events = events, .data = {.ptr = serialized}};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serialized->getFd(), &new_event);
    }

    // Later will be std::vector<AbstractSerialized*>
    AbstractSerialized *wait()
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

private:
    void handleIO(AbstractSerialized& serialized, uint32_t events)
    {
        if (events & EPOLLIN) {
            serialized.handleIn();
        }
        if (events & EPOLLOUT) {
            serialized.handleOut();
        }
    }

    void updateSerializedMode(AbstractSerialized& serialized) const
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

    int epoll_fd = 0;
};
