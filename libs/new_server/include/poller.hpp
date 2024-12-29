#pragma once

#include "abstract_serialized.hpp"
#include "logger.hpp"

class Poller
{
public:
    Poller();
    void addSerialized(AbstractSerialized *serialized);
    // Later will be std::vector<AbstractSerialized*>
    AbstractSerialized *wait();

private:
    void handleIO(AbstractSerialized& serialized, uint32_t events) const;
    void updateSerializedMode(AbstractSerialized& serialized) const;

    int epoll_fd = 0;
    Logger logger{"Poller"};
};
