#include "startup.hpp"
#include "backend_service.hpp"
#include "frontend_service.hpp"
#include "server.hpp"
#include <iostream>

Startup::Startup(int argc, char *argv[])  // NOLINT (cppcoreguidelines-avoid-c-arrays)
{
    namespace opt = boost::program_options;
    desc.add_options()("resource-dir", opt::value<std::string>(&resource_dir), "directory with resources");
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
    server = std::make_unique<Server>(backend->getLogicalController());

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
    if (resource_dir.empty()) {
        try {
            opt::store(opt::parse_config_file<char>("kserver.cfg", desc), vm);
            opt::notify(vm);
        }
        catch (const opt::reading_file& e) {
            std::cout << "Error: " << e.what() << std::endl;
            std::exit(1);
        }
    }
}

void Startup::handleHelp() const
{
    if (vm.count("help") != 0) {
        std::cout << desc << std::endl;
        std::exit(0);
    }
}
