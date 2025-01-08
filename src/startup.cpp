#include "startup.hpp"
#include "backend_service.hpp"
#include "frontend_service.hpp"
#include "server.hpp"
#include <iostream>

Startup::Startup(int argc, char *argv[])  // NOLINT (cppcoreguidelines-avoid-c-arrays)
{
    namespace opt = boost::program_options;
    desc.add_options()                                                                        //
        ("resource-dir", opt::value<std::string>(&resource_dir), "directory with resources")  //
        ("port", opt::value<int>(&port), "port for server");
    opt::store(opt::parse_command_line(argc, argv, desc), vm);
    opt::notify(vm);
}

void Startup::start()
{
    initParams();
    frontend = std::make_shared<FrontendService>(resource_dir);
    backend = std::make_shared<BackendService>();
    backend->setFrontendService(frontend);
    backend->init();
    server = std::make_unique<Server>(backend->getLogicalController(), frontend, port);

    run();
}

void Startup::run()
{
    try {
        server->run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        std::exit(1);
    }
}

void Startup::initParams()
{
    handleHelp();
    namespace opt = boost::program_options;
    if (resource_dir.empty() || port == -1) {
        logger.log("Read params from config file");
        try {
            opt::store(opt::parse_config_file<char>("kserver.cfg", desc), vm);
            opt::notify(vm);
        }
        catch (const opt::reading_file& e) {
            std::cout << "Error: " << e.what() << std::endl;
            std::exit(1);
        }
    }
    logger.log("Params:", "resource_dir: " + resource_dir + " port: " + std::to_string(port));
}

void Startup::handleHelp() const
{
    if (vm.count("help") != 0) {
        std::cout << desc << std::endl;
        std::exit(0);
    }
}
