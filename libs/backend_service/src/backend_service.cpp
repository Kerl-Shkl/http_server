#include "backend_service.hpp"
#include "database.hpp"
#include "frontend_service.hpp"
#include "logical_controller.hpp"
#include "md4c-html.h"

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
    controller->addAction(  //
        HttpMethod::GET, "/api/note_body", [this](const HttpRequest& request) -> HttpResponse {
            HttpResponse response;
            const auto& headers = request.getHeaders();
            if (auto it = headers.find("id"); it != headers.end()) {
                int id = std::stoi(it->second);
                std::string body = note(id);
                response.setStatus("OK");
                response.setCode(200);
                response.setBody("text/html", std::move(body));
            }
            else {
                response.setCode(400);
                response.setStatus("Bad Request");
            }
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
    std::string html;
    auto process = [](const MD_CHAR *text, MD_SIZE size, void *html_string) -> void {
        auto& html = *static_cast<std::string *>(html_string);
        html.append(text, size);
    };
    md_html(md.data(), md.size(), process, &html, 0, 1);
    return html;
}
