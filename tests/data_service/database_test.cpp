#include "database.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <pqxx/pqxx>

constexpr auto connection_string = "postgresql://kerl@/test_notes";
constexpr auto section_name = "test section name";
constexpr auto note_name = "test note name";
constexpr auto note_body = "test note body";

static void clearTestDB()
{
    pqxx::connection connection{connection_string};
    pqxx::work transaction{connection};
    transaction.exec("DELETE FROM notes WHERE name = '" + transaction.esc(note_name) + "';");
    transaction.exec("DELETE FROM sections WHERE name = '" + transaction.esc(section_name) + "';");
    transaction.commit();
}

class DBCleaner : public ::testing::Environment
{
public:
    void TearDown() override
    {
        clearTestDB();
    }
};

class TestDB : public testing::Test
{
protected:
    void SetUp() override
    {
        database = std::make_unique<DataBase>(connection_string);
    }

    void TearDown() override
    {
        clearTestDB();
    }

    std::unique_ptr<DataBase> database;
};

TEST_F(TestDB, addAndGetSection)
{
    int section_id = database->addSection(section_name);
    auto selected_name = database->getSection(section_id);

    EXPECT_EQ(section_name, selected_name);
}

TEST_F(TestDB, getNonExistentSection)
{
    int nonexistent_id = 42;

    auto selected_name = database->getSection(nonexistent_id);

    EXPECT_TRUE(selected_name.empty());
}

TEST_F(TestDB, addAndDeleteSection)
{
    int section_id = database->addSection(section_name);
    database->deleteSection(section_name);
    auto selected_name = database->getSection(section_id);

    EXPECT_TRUE(selected_name.empty());
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
