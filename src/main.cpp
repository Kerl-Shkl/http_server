#include "server.hpp"
#include <iostream>

int main()
{
    try {
        LogicalController logical_controller;
        Server server{std::move(logical_controller)};
        server.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
