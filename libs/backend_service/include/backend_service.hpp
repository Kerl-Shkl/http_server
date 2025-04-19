#pragma once

#include "nlohmann/json.hpp"
#include <memory>

class LogicalController;
class FrontendService;
class DataBase;
class PermissionsController;

class BackendService
{
public:
    BackendService();
    ~BackendService();

    void init();
    void setFrontendService(std::shared_ptr<FrontendService> frontend);
    std::shared_ptr<LogicalController> getLogicalController();
    PermissionsController& getPermissionsController();

private:
    using json = nlohmann::json;
    void addPageAction(const std::string& target, const std::string_view resource);
    json noteNamesList();
    std::string note(int id);
    void addNote(std::string name, std::string note_body, std::string section_name);

    std::shared_ptr<LogicalController> controller;
    std::shared_ptr<FrontendService> frontend;
    std::unique_ptr<DataBase> database;
    std::unique_ptr<PermissionsController> permissions_controller;
};
