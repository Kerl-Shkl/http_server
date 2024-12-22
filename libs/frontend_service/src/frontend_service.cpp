#include "frontend_service.hpp"
#include <fstream>

FrontendService::FrontendService()
{
    collectIndex();
}

FrontendService& FrontendService::operator=(FrontendService&& rhp) noexcept
{
    if (&rhp == this) {
        return *this;
    }
    resource_index = std::move(rhp.resource_index);
    return *this;
}

std::pair<std::string, std::string> FrontendService::getContent(const std::string_view target) const
{
    auto corrected_target = target.starts_with("/") ? target.substr(1) : target;
    if (auto it = resource_index.find(std::string{corrected_target}); it != resource_index.end()) {
        std::ifstream resource(it->second);
        std::string content((std::istreambuf_iterator<char>(resource)), (std::istreambuf_iterator<char>()));
        return {extensionToContentType(it->second.extension()), content};
    }
    return {"", ""};
}

void FrontendService::collectIndex()
{
    namespace fs = std::filesystem;
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(resource_dir)) {
        if (dir_entry.is_regular_file()) {
            auto path = dir_entry.path();
            resource_index.emplace(path.filename(), path);
        }
    }
}

std::string FrontendService::extensionToContentType(const std::string& extension) const
{
    if (auto it = extensions_map.find(extension); it != extensions_map.end()) {
        return it->second;
    }
    return "";
}
