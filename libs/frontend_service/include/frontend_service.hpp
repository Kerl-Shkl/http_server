#pragma once

#include "functor_utils.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

#include "resource_observer.hpp"

class FrontendService
{
public:
    FrontendService(std::string resource_dir);
    ~FrontendService() = default;
    FrontendService(FrontendService&&) = default;
    FrontendService& operator=(FrontendService&&) = delete;
    FrontendService(const FrontendService&) = delete;
    FrontendService& operator=(const FrontendService&) = delete;

    std::pair<std::string, std::string> getContent(const std::string_view target) const;
    ResourceObserver& getResourceObserver();
    std::string extensionToContentType(const std::string_view extension) const;

private:
    void collectIndex();
    void loadExtensions();

    std::filesystem::path resource_dir;
    ResourceObserver resource_observer;
    std::filesystem::path extension_file;
    std::unordered_map<std::string, std::string, StringViewHash, StringViewEqualTo> extensions_map;
    Logger logger{"FrontendService"};
};
