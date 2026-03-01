#include "pq_connection.hpp"
#include "pq_database.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>

constexpr auto connection_string = "postgresql://kerl@/test_notes";
constexpr auto section_name = "test section name";
constexpr auto note_name = "test note name";
constexpr auto note_body = "test note body";
constexpr auto note_name2 = "second test note name";
constexpr auto note_body2 = "second test note body";

namespace {

void clearTestDB()
{
    PQConnection connection{connection_string};
    connection.connect();
    connection.exec("DELETE FROM notes WHERE name = $1 OR name = $2;", note_name, note_name2);
    connection.exec("DELETE FROM sections WHERE name = $1;", section_name);
}

bool isDBClean()
{
    PQConnection connection{connection_string};
    connection.connect();
    auto notes_count = connection.exec("SELECT COUNT(*) FROM notes;");
    auto sections_count = connection.exec("SELECT COUNT(*) FROM sections;");
    return notes_count.getOnlyOne<int>() == 0 && sections_count.getOnlyOne<int>() == 0;
}

}  // namespace

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
        database = std::make_unique<PostgresDB>(connection_string);
    }

    void TearDown() override
    {
        clearTestDB();
    }

    std::unique_ptr<IDataBase> database;
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
    constexpr std::string_view nonexistent_name = "nonexistent_name";

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

    EXPECT_EQ(note_body, body_by_id);
}

TEST_F(TestDB, getNonExistentNote)
{
    int nonexistent_id = 42;

    auto body = database->getNote(nonexistent_id);

    EXPECT_TRUE(body.empty());
}

TEST_F(TestDB, addAndGetNoteWithName)
{
    int note_id = database->addNote(note_name, note_body);
    auto [selected_name, selected_body] = database->getNoteWithName(note_id);

    EXPECT_EQ(note_name, selected_name);
    EXPECT_EQ(note_body, selected_body);
}

TEST_F(TestDB, addNoteWithSection)
{
    int section_id = database->addSection(section_name);
    int note_id = database->addNote(note_name, note_body, section_id);
    auto body = database->getNote(note_name);
    auto selected_section = database->getSectionIdByNote(note_id);

    EXPECT_EQ(note_body, body);
    EXPECT_EQ(*selected_section, section_id);
}

TEST_F(TestDB, addNoteWithSectionName)
{
    int section_id = database->addSection(section_name);
    int note_id = database->addNote(note_name, note_body, section_name);
    auto body = database->getNote(note_name);
    auto selected_section = database->getSectionIdByNote(note_id);

    EXPECT_EQ(note_body, body);
    EXPECT_EQ(*selected_section, section_id);
}

TEST_F(TestDB, addAndDeleteNoteByName)
{
    int note_id = database->addNote(note_name, note_body);
    database->deleteNote(note_name);
    auto body = database->getNote(note_name);

    EXPECT_TRUE(body.empty());
}

TEST_F(TestDB, addAndDeleteNoteById)
{
    int note_id = database->addNote(note_name, note_body);
    database->deleteNote(note_id);
    auto body = database->getNote(note_name);

    EXPECT_TRUE(body.empty());
}

TEST_F(TestDB, addNoteAndSection)
{
    int note_id = database->addNote(note_name, note_body, section_name);
    auto body = database->getNote(note_name);
    auto selected_section = database->getSectionIdByNote(note_id);

    EXPECT_EQ(note_body, body);
    EXPECT_TRUE(selected_section.has_value());
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
    EXPECT_TRUE(std::ranges::is_permutation(note_names, expected));
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
