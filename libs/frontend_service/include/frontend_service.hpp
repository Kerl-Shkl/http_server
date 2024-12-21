#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <gtest/gtest.h>

class FrontendService
{
public:
    FrontendService();
    std::pair<std::string, std::string> getContent(const std::string_view target) const;

private:
    void collectIndex();
    std::string extensionToContentType(const std::string& extension) const;

    std::unordered_map<std::string, std::filesystem::path> resource_index;
    static constexpr std::string_view resource_dir = "/home/kerl/work/C++/Server/html_css";
    const std::unordered_map<std::string, std::string> extensions_map{
        {"html", "text/html" },
        {"jpeg", "image/jpeg"},
        {"jpg",  "image/jpeg"},
        {"png",  "image/png" }
    };

    FRIEND_TEST(FrontendService, CollectIndex);
};