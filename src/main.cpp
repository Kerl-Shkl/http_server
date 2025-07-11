#include "startup.hpp"

int main(int argc, char *argv[])
{
    Startup startup{argc, argv};
    startup.start();
}
