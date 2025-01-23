#include "database.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <pqxx/pqxx>

constexpr auto connection_string = "postgresql://kerl@/test_notes";
constexpr auto section_name = "test section name";
constexpr auto note_name = "test note name";
constexpr auto note_body = "test note body";
constexpr auto note_name2 = "second test note name";
constexpr auto note_body2 = "second test note body";

static void clearTestDB()
{
    pqxx::connection connection{connection_string};
    pqxx::work transaction{connection};
    transaction.exec("DELETE FROM notes WHERE name = '" + transaction.esc(note_name) + "';");
    transaction.exec("DELETE FROM notes WHERE name = '" + transaction.esc(note_name2) + "';");
    transaction.exec("DELETE FROM sections WHERE name = '" + transaction.esc(section_name) + "';");
    transaction.commit();
}

static bool isDBClean()
{
    pqxx::connection connection{connection_string};
    pqxx::work transaction{connection};
    pqxx::row notes_count = transaction.exec1("SELECT COUNT(*) FROM notes;");
    pqxx::row sections_count = transaction.exec1("SELECT COUNT(*) FROM sections;");
    return notes_count[0].as<int>() == 0 && sections_count[0].as<int>() == 0;
}

class DBCleaner : public ::testing::Environment
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(isDBClean());
    }
    void TearDown() override
    {
        clearTestDB();
        ASSERT_TRUE(isDBClean());
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

TEST_F(TestDB, addAndGetNote)
{
    int note_id = database->addNote(note_name, note_body);
    auto body_by_id = database->getNote(note_id);
    auto body_by_name = database->getNote(note_name);

    EXPECT_EQ(note_body, body_by_id);
    EXPECT_EQ(note_body, body_by_name);
}

TEST_F(TestDB, getNonExistentNote)
{
    int nonexistent_id = 42;

    auto body = database->getNote(nonexistent_id);

    EXPECT_TRUE(body.empty());
}

TEST_F(TestDB, addNoteWithSection)
{
    int section_id = database->addSection(section_name);
    int note_id = database->addNote(note_name, note_body, section_id);
    auto body = database->getNote(note_name);

    EXPECT_EQ(note_body, body);
}

TEST_F(TestDB, addAndDeleteNote)
{
    int note_id = database->addNote(note_name, note_body);
    database->deleteNote(note_name);
    auto body = database->getNote(note_name);

    EXPECT_TRUE(body.empty());
}

TEST_F(TestDB, getAllNotes)
{
    int id = database->addNote(note_name, note_body);
    int id2 = database->addNote(note_name2, note_body2);
    auto note_names = database->getAllNoteNames();

    std::vector<std::pair<int, std::string>> expected{
        {id,  note_name },
        {id2, note_name2}
    };
    EXPECT_TRUE(std::is_permutation(note_names.begin(), note_names.end(), expected.begin(), expected.end()));
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
