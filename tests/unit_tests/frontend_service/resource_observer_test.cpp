#include "resource_observer.hpp"
#include <gtest/gtest.h>
#include <memory>

class TrackedDirectory : public testing::Test
{
protected:
    void SetUp() override
    {
        bool created = std::filesystem::create_directory(sandbox);
        ASSERT_TRUE(created);
        created = std::filesystem::create_directory(start);
        ASSERT_TRUE(created);
        resource_observer = std::make_unique<ResourceObserver>(std::string{start});
        ASSERT_EQ(resource_observer->getTrackedDirs().size(), 1);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(sandbox);
    }

    static constexpr std::string_view sandbox_path = "./sandbox";
    static constexpr std::string_view start_path = "./start";
    const std::filesystem::path sandbox{sandbox_path};
    const std::filesystem::path start{sandbox / start_path};
    std::unique_ptr<ResourceObserver> resource_observer;
};

TEST_F(TrackedDirectory, addNewDirectory)
{
    std::filesystem::path new_dir = start / "new_dir_with_very_long_name_and_path_take_big_buffer";
    bool created = std::filesystem::create_directory(new_dir);
    ASSERT_TRUE(created);

    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 2);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(new_dir));
}

TEST_F(TrackedDirectory, addNewDirectoryWithSubdirectory)
{
    std::filesystem::path new_dir = start / "new_dir";
    std::filesystem::path new_subdir = new_dir / "new_subdir";
    bool created = std::filesystem::create_directory(new_dir);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(new_subdir);
    ASSERT_TRUE(created);

    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 3);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(new_dir));
    EXPECT_TRUE(dirs.contains(new_subdir));
}

TEST_F(TrackedDirectory, deleteDirectoryWithSubdirectory)
{
    std::filesystem::path new_dir = start / "new_dir";
    std::filesystem::path new_subdir = new_dir / "new_subdir";
    std::filesystem::path second_subdir = new_dir / "second_subdir";
    bool created = std::filesystem::create_directory(new_dir);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(new_subdir);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(second_subdir);
    ASSERT_TRUE(created);

    resource_observer->handleIn();
    std::filesystem::remove_all(new_dir);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 1);
    EXPECT_TRUE(dirs.contains(start));
}

TEST_F(TrackedDirectory, moveInSandbox)
{
    std::filesystem::path source = "./new_dir";
    std::filesystem::path inner = source / "inner";
    std::filesystem::path destination = start / "new_dir";
    std::filesystem::path updated_inner = destination / "inner";
    bool created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(inner);
    ASSERT_TRUE(created);

    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 3);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(destination));
    EXPECT_TRUE(dirs.contains(updated_inner));
}

TEST_F(TrackedDirectory, moveFromSandbox)
{
    std::filesystem::path source = start / "new_dir";
    std::filesystem::path inner = source / "inner";
    std::filesystem::path destination = sandbox / "destination";
    bool created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(inner);
    ASSERT_TRUE(created);

    resource_observer->handleIn();
    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 1);
    EXPECT_TRUE(dirs.contains(start));
}

TEST_F(TrackedDirectory, moveFromAndInSandbox)
{
    std::filesystem::path sub = start / "new_dir";
    std::filesystem::path source = sub / "inner";
    std::filesystem::path destination = start / "destination";
    bool created = std::filesystem::create_directory(sub);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);

    resource_observer->handleIn();
    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 3);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(sub));
    EXPECT_TRUE(dirs.contains(destination));
}
