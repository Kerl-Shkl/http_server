#pragma once

#include "poller.hpp"
#include <cstdint>

class AbstractSerialized
{
public:
    virtual ~AbstractSerialized() = default;
    virtual void handleIn() = 0;
    virtual void handleOut() = 0;

    [[nodiscard]] virtual int getFd() const = 0;
    [[nodiscard]] virtual bool wantIn() const = 0;
    [[nodiscard]] virtual bool wantOut() const = 0;

protected:
    void updateSerializedMode()
    {
        poller->updateSerializedMode(this);
    }

private:
    std::uint32_t actual_events = 0;
    Poller *poller{nullptr};
    friend class Poller;
};
