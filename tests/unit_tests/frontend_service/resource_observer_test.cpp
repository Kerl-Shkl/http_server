#include "resource_observer.hpp"
#include "poller.hpp"
#include <gtest/gtest.h>

int main()
{
    ResourceObserver resource_observer{"sandbox"};
    Poller poller;
    poller.addSerialized(&resource_observer);
    for (;;) {
        resource_observer.print();
        poller.wait();
    }
}
