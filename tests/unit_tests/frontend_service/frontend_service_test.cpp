#include "frontend_service.hpp"
#include <fstream>
#include <gtest/gtest.h>

#include <iostream>

class FrontendServiceFixture : public testing::Test
{
protected:
    void SetUp() override
    {
        bool created = std::filesystem::create_directory(sandbox);
        ASSERT_TRUE(created);
        std::ofstream extensions{sandbox / "extensions"};
        extensions << "html text/html" << "\n";
        extensions << "css text/css" << "\n";
        extensions.close();
        frontend_service = std::make_unique<FrontendService>(std::string{sandbox_path});
    }

    void TearDown() override
    {
        std::filesystem::remove_all(sandbox);
    }

    static constexpr std::string_view sandbox_path = "./sandbox";
    const std::filesystem::path sandbox{sandbox_path};
    std::unique_ptr<FrontendService> frontend_service;
};

TEST_F(FrontendServiceFixture, extensionsLoad)
{
    std::string content_type = frontend_service->extensionToContentType("html");
    EXPECT_EQ(content_type, "text/html");
}

TEST_F(FrontendServiceFixture, extensionsUpdate)
{
    std::string content_type = frontend_service->extensionToContentType("kerl");
    EXPECT_EQ(content_type, "");
    std::ofstream{sandbox / "extensions"} << "kerl shkl";
    frontend_service->getResourceObserver().handleIn();
    content_type = frontend_service->extensionToContentType("kerl");
    EXPECT_EQ(content_type, "shkl");
}

TEST_F(FrontendServiceFixture, getContent)
{
    std::string body = "resource body";
    std::ofstream{sandbox / "resource.html"} << body;

    frontend_service->getResourceObserver().handleIn();
    std::cout << frontend_service->getResourceObserver().getResourcePath("/resource.html") << std::endl;
    auto [type, content] = frontend_service->getContent("/resource.html");

    EXPECT_EQ(type, "text/html");
    EXPECT_EQ(content, body);
}
