#include "frontend_service.hpp"
#include <fstream>

FrontendService::FrontendService(std::string res_dir)
: resource_dir{std::move(res_dir)}
, resource_observer{resource_dir}
, extension_file{resource_dir / "extensions"}
{
    resource_observer.trackChangesInFile(extension_file, [this]() { loadExtensions(); });
    loadExtensions();
}

std::pair<std::string, std::string> FrontendService::getContent(const std::string_view target) const
{
    auto corrected_target = target.starts_with("/") ? target.substr(1) : target;
    auto resource_path = resource_observer.getResourcePath(target);
    if (!resource_path.empty()) {
        std::ifstream resource(resource_path);
        std::string content((std::istreambuf_iterator<char>(resource)), (std::istreambuf_iterator<char>()));
        logger.log("extensions: ", resource_path.extension());
        return {extensionToContentType(resource_path.extension().string()), content};
    }
    return {"", ""};
}

ResourceObserver& FrontendService::getResourceObserver()
{
    return resource_observer;
}

std::string FrontendService::extensionToContentType(const std::string_view extension) const
{
    std::string_view corrected_extension = extension.starts_with('.') ? extension.substr(1) : extension;
    if (auto it = extensions_map.find(corrected_extension); it != extensions_map.end()) {
        return it->second;
    }
    return "";
}

void FrontendService::loadExtensions()
{
    std::ifstream finput{extension_file};
    if (!finput.is_open()) {
        throw std::runtime_error{"Can't open extension file: " + extension_file.string()};
    }
    std::string extension;
    std::string content_type;
    while (finput >> extension >> content_type) {
        extensions_map.emplace(std::move(extension), std::move(content_type));
    }
}
