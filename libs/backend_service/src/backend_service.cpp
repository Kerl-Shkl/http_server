#include "backend_service.hpp"
#include "database.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"

BackendService::BackendService()
: controller{std::make_shared<LogicalController>()}
{
    database = std::make_unique<DataBase>("postgresql://kerl@/notes");
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

auto BackendService::noteNamesList() -> json
{
    auto note_names = database->getAllNoteNames();
    json result = std::move(note_names);
    return result;
}
