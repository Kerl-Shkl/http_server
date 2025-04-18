#pragma once

#include "abstract_serialized.hpp"
#include "logger.hpp"

class Poller
{
public:
    Poller();
    ~Poller();
    void addSerialized(AbstractSerialized *serialized);
    // Later will be std::vector<AbstractSerialized*>
    AbstractSerialized *wait();
    AbstractSerialized *check();  // For tests only
    void updateSerializedMode(AbstractSerialized& serialized) const;

private:
    void handleIO(AbstractSerialized& serialized, uint32_t events) const;

    int epoll_fd = 0;
    Logger logger{"Poller"};
};
