#pragma once
#include <memory>

class LogicalController;
class FrontendService;

class BackendService
{
public:
    BackendService();
    ~BackendService();

    void init();
    void setFrontendService(std::shared_ptr<FrontendService> frontend);
    std::shared_ptr<LogicalController> getLogicalController();

private:
    std::shared_ptr<LogicalController> controller;
    std::shared_ptr<FrontendService> frontend;
};
