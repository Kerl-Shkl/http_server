#include "backend_service.hpp"
#include "database.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"
#include "md4c-html.h"
#include "permissions_controller.hpp"

BackendService::BackendService()
: controller{std::make_shared<LogicalController>()}
, permissions_controller(std::make_unique<PermissionsController>())
{
    database = std::make_unique<DataBase>("postgresql://kerl@/notes");
    permissions_controller->startBotCommunication();
}

BackendService::~BackendService() = default;

void BackendService::setFrontendService(std::shared_ptr<FrontendService> frontend_service)
{
    frontend = std::move(frontend_service);
    controller->setFrontendService(frontend);
}

void BackendService::init()
{
    addPageAction("/home", "cv.html");
    addPageAction("/faq", "faq.html");
    addPageAction("/notes", "note.html");
    controller->addAction(  //
        HttpMethod::GET, "/api/note_names", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            json note_names = noteNamesList();
            response.setStatus("OK");
            response.setCode(200);
            response.setBody("application/json", note_names.dump());
            return response;
        });
    controller->addAction(  //
        HttpMethod::GET, "/api/note_body", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            const auto& headers = request.getHeaders();
            if (auto it = headers.find("id"); it != headers.end()) {
                try {
                    int id = std::stoi(it->second);
                    std::string body = note(id);
                    response.setStatus("OK");
                    response.setCode(200);
                    response.setBody("text/html", std::move(body));
                }
                catch (const std::exception& ex) {
                    response.setCode(400);
                    response.setStatus("Bad Request");
                }
            }
            else {
                response.setCode(400);
                response.setStatus("Bad Request");
            }
            return response;
        });
    controller->addAction(  //
        HttpMethod::GET, "/api/raw_note", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            const auto& headers = request.getHeaders();
            if (auto it = headers.find("id"); it != headers.end()) {
                try {
                    int id = std::stoi(it->second);
                    auto [name, body] = database->getNoteWithName(id);
                    response.setStatus("OK");
                    response.setCode(200);
                    response.addHeader("name", std::move(name));
                    response.setBody("text/plain", std::move(body));
                }
                catch (const std::exception& ex) {
                    response.setCode(400);
                    response.setStatus("Bad Request");
                }
            }
            else {
                response.setCode(400);
                response.setStatus("Bad Request");
            }
            return response;
        });
    controller->addAction(  //
        HttpMethod::POST, "/api/add_note", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            try {
                json info = json::parse(request.getBody());
                std::string name = std::move(info["name"]);
                std::string section_name = std::move(info["section_name"]);
                std::string note_body = std::move(info["body"]);
                addNote(std::move(name), std::move(note_body), std::move(section_name));
            }
            catch (const std::exception& ex) {
                logger.log("Exception in handling addNote request ", ex.what());
                response.setCode(400);
                response.setStatus("Bad Request");
                return response;
            }
            response.setCode(200);
            response.setStatus("OK");
            return response;
        });
    controller->addAction(  //
        HttpMethod::DELETE, "/api/delete_note", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            const auto& headers = request.getHeaders();
            if (auto it = headers.find("id"); it != headers.end()) {
                int id{};
                try {
                    id = std::stoi(it->second);
                }
                catch (const std::exception& ex) {
                    response.setCode(400);
                    response.setStatus("Bad Request");
                    return response;
                }
                try {
                    deleteNote(id);
                    response.setCode(200);
                    response.setStatus("OK");
                    return response;
                }
                catch (const std::runtime_error& ex) {
                    response.setCode(400);
                    response.setStatus("Bad Request");
                    return response;
                }
                catch (const std::exception& ex) {
                    response.setCode(507);
                    response.setStatus("Insufficient Storage");
                    return response;
                }
            }
            else {
                response.setCode(400);
                response.setStatus("Bad Request");
                return response;
            }
        });
}

void BackendService::addPageAction(const std::string& target, const std::string_view resource)
{
    controller->addAction(  //
        HttpMethod::GET, target, [this, resource](const HttpRequest&) -> HttpResponse {
            auto [content_type, content] = frontend->getContent(resource);
            assert(!content.empty());
            HttpResponse response;
            response.setBody(std::move(content_type), std::move(content));
            response.setStatus("OK");
            response.setCode(200);
            return response;
        });
}

std::shared_ptr<LogicalController> BackendService::getLogicalController()
{
    return controller;
}

PermissionsController& BackendService::getPermissionsController()
{
    return *permissions_controller;
}

auto BackendService::noteNamesList() -> json
{
    auto note_names = database->getAllNoteNames();
    json result = {};
    for (auto& [id, name] : note_names) {
        json element;
        element["id"] = id;
        element["name"] = std::move(name);
        result.push_back(std::move(element));
    }
    return result;
}

std::string BackendService::note(int id)
{
    std::string md = database->getNote(id);
    std::pair<std::stringstream, bool> process_field;
    auto process = [](const MD_CHAR *text, MD_SIZE size, void *field) -> void {
        auto& [out_html, display_opened] = *static_cast<std::pair<std::stringstream, bool> *>(field);
        std::string_view new_batch{text, size};
        if (new_batch == "<x-equation>") {
            display_opened = false;
            out_html << "$";
        }
        else if (new_batch == R"(<x-equation type="display">)") {
            display_opened = true;
            out_html << "$$";
        }
        else if (new_batch == "</x-equation>") {
            out_html << (display_opened ? "$$" : "$");
        }
        else {
            out_html << new_batch;
        }
    };
    md_html(md.data(), md.size(), process, &process_field, MD_FLAG_LATEXMATHSPANS, 1);
    return process_field.first.str();
}

void BackendService::addNote(std::string name, std::string note_body, std::string section_name)
{
    auto deffered_action = [this, name, note_body = std::move(note_body),
                            section_name = std::move(section_name)](bool allowed)  //
    {
        if (allowed) {
            try {
                database->addNote(name, note_body, section_name);
            }
            catch (const std::exception& ex) {
                logger.log("Exception in deffered addNote action ", ex.what());
            }
        }
    };
    permissions_controller->askPermission(std::move(name), RequestOperation::add, std::move(deffered_action));
}

void BackendService::deleteNote(int id)
{
    std::string note_name = database->getNoteName(id);
    if (note_name.empty()) {
        throw std::runtime_error{"There is no note with such id"};
    }
    auto deffered_action = [this, id](bool allowed) {
        if (allowed) {
            try {
                database->deleteNote(id);
            }
            catch (const std::exception& ex) {
                logger.log("Exception in deffered deleteNote action ", ex.what());
            }
        }
    };
    permissions_controller->askPermission(std::move(note_name), RequestOperation::remove,
                                          std::move(deffered_action));
}
