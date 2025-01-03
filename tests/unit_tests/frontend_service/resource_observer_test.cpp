#include "resource_observer.hpp"
#include <fstream>
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
    std::string filename = "file.txt";
    bool created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(inner);
    ASSERT_TRUE(created);
    std::ofstream{inner / filename}.put('a');

    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 3);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(destination));
    EXPECT_TRUE(dirs.contains(updated_inner));
    EXPECT_EQ(resource_observer->getResourcePath("/" + filename), updated_inner / filename);
}

TEST_F(TrackedDirectory, moveFromSandbox)
{
    std::filesystem::path source = start / "new_dir";
    std::filesystem::path inner = source / "inner";
    std::filesystem::path destination = sandbox / "destination";
    std::string filename = "file.txt";
    bool created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(inner);
    ASSERT_TRUE(created);
    std::ofstream{source / filename}.put('a');

    resource_observer->handleIn();
    ASSERT_EQ(resource_observer->getResourcePath(filename), source / filename);
    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 1);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_EQ(resource_observer->getResourcePath(filename), std::filesystem::path(""));
}

TEST_F(TrackedDirectory, moveFromAndInSandbox)
{
    std::filesystem::path sub = start / "new_dir";
    std::filesystem::path source = sub / "inner";
    std::filesystem::path destination = start / "destination";
    std::string filename = "file.txt";
    bool created = std::filesystem::create_directory(sub);
    ASSERT_TRUE(created);
    created = std::filesystem::create_directory(source);
    ASSERT_TRUE(created);
    std::ofstream{source / filename}.put('a');

    resource_observer->handleIn();
    std::filesystem::rename(source, destination);
    resource_observer->handleIn();

    auto dirs = resource_observer->getTrackedDirs();
    EXPECT_EQ(dirs.size(), 3);
    EXPECT_TRUE(dirs.contains(start));
    EXPECT_TRUE(dirs.contains(sub));
    EXPECT_TRUE(dirs.contains(destination));
    EXPECT_EQ(resource_observer->getResourcePath(filename), destination / filename);
}

TEST_F(TrackedDirectory, trackModify)
{
    std::filesystem::path file = sandbox / "file.txt";
    std::ofstream{file}.put('a');

    int counter{0};
    resource_observer->trackChangesInFile(file, [&counter]() { ++counter; });
    std::ofstream{file}.put('b');
    resource_observer->handleIn();

    EXPECT_EQ(counter, 1);
}

TEST_F(TrackedDirectory, trackDelete)
{
    std::filesystem::path file = sandbox / "file.txt";
    std::ofstream{file}.put('a');
    int counter{0};
    struct Handler
    {
        Handler(int& c)
        : counter(c)
        {}
        ~Handler()
        {
            ++counter;
        }
        void operator()()
        {}
        int& counter;
    };

    Handler handler{counter};
    resource_observer->trackChangesInFile(file, handler);
    std::filesystem::remove(file);
    resource_observer->handleIn();

    EXPECT_EQ(counter, 1);
}

TEST_F(TrackedDirectory, untrackFile)
{
    std::filesystem::path file = sandbox / "file.txt";
    std::ofstream{file}.put('a');

    int counter{0};
    int id = resource_observer->trackChangesInFile(file, [&counter]() { ++counter; });
    std::ofstream{file}.put('b');
    resource_observer->handleIn();
    resource_observer->untrackFile(id);
    std::ofstream{file}.put('c');
    resource_observer->handleIn();

    EXPECT_EQ(counter, 1);
}

TEST_F(TrackedDirectory, fileCreationAndDeletion)
{
    std::filesystem::path file = start / "file.txt";
    std::ofstream{file}.put('a');

    resource_observer->handleIn();
    EXPECT_EQ(resource_observer->getResourcePath("/file.txt"), file);
    std::filesystem::remove(file);
    resource_observer->handleIn();
    EXPECT_EQ(resource_observer->getResourcePath("/file.txt"), "");
}
