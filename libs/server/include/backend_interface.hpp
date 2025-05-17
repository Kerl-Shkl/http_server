#pragma once

#include <memory>

class LogicalController;

class BackendInterface
{
public:
    virtual ~BackendInterface() = default;
    virtual void init(std::shared_ptr<LogicalController> controller) = 0;
};
