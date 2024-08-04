#include "server.hpp"
#include <iostream>

int main()
{
    try {
        Server server;
        server.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
