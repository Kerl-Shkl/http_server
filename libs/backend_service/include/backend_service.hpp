#pragma once

#include "backend_interface.hpp"
#include "logger.hpp"
#include "nlohmann/json.hpp"
#include <memory>

class DataBase;
class PermissionsController;

class BackendService : public BackendInterface
{
public:
    BackendService();
    ~BackendService() override;

    void init(std::shared_ptr<LogicalController> controller) override;
    std::shared_ptr<LogicalController> getLogicalController();
    PermissionsController& getPermissionsController();

private:
    using json = nlohmann::json;
    json noteNamesList();
    std::string note(int id);
    void addNote(std::string name, std::string note_body, std::string section_name);
    void deleteNote(int id);

    std::shared_ptr<LogicalController> controller;
    std::unique_ptr<DataBase> database;
    std::unique_ptr<PermissionsController> permissions_controller;
    Logger logger{"BackendService"};
};
