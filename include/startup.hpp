#pragma once

#include "server.hpp"
#include <boost/program_options.hpp>

class BackendService;
class FrontendService;
class Server;

class Startup
{
public:
    Startup(int argc, char *argv[]);  // NOLINT (cppcoreguidelines-avoid-c-arrays)
    void start();

private:
    void handleHelp() const;
    void initParams();
    void run();

    boost::program_options::variables_map vm;
    boost::program_options::options_description desc{"All options"};
    std::string resource_dir;
    int port{-1};
    std::shared_ptr<BackendService> backend;
    std::shared_ptr<FrontendService> frontend;
    std::unique_ptr<Server> server;

    Logger logger{"Startup"};
};
