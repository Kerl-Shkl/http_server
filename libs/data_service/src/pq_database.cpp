#include "pq_database.hpp"
#include "pq_connection.hpp"
#include "pq_utils.hpp"

PostgresDB::PostgresDB(std::string conn_string)
: connection{std::make_unique<PQConnection>(std::move(conn_string))}
{
    connection->connect();
}
PostgresDB::~PostgresDB() = default;

int PostgresDB::addSection(const std::string& section_name)
{
    ResultWrapper result =
        connection->exec("INSERT INTO sections (name) VALUES ($1) RETURNING id", section_name);
    if (!result.valid() || result.empty()) {
        throw std::runtime_error{"Error while adding section " + section_name + " " +
                                 std::to_string(result.status())};
    }
    return result.getOnlyOne<int>();
}

void PostgresDB::deleteSection(const std::string& section_name)
{
    ResultWrapper result = connection->exec("DELETE FROM sections WHERE name = $1;", section_name);
    if (!result.valid()) {
        throw std::runtime_error{"Error while deleting section " + section_name + " " +
                                 std::to_string(result.status())};
    }
}

std::string PostgresDB::getSection(int id)
{
    ResultWrapper result = connection->exec("SELECT name FROM sections WHERE id = $1;", id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select section " + std::to_string(id) + " " +
                                 std::to_string(result.status())};
    }
    if (result.empty()) {
        return "";
    }
    return result.getOnlyOne<std::string>();
}

int PostgresDB::getOrAddSection(const std::string& section_name)
{
    ResultWrapper result = connection->exec("SELECT id FROM sections WHERE name = $1;", section_name);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select section " + section_name + " " +
                                 std::to_string(result.status())};
    }
    if (!result.empty()) {
        return result.getOnlyOne<int>();
    }
    return addSection(section_name);
}

std::optional<int> PostgresDB::getSectionIdByNote(int note_id)
{
    ResultWrapper result = connection->exec("SELECT section_id FROM notes WHERE id = $1;", note_id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select section_id via note id " + std::to_string(note_id) +
                                 " " + std::to_string(result.status())};
    }
    if (result.empty()) {
        return std::nullopt;
    }
    return result.getOnlyOne<int>();
}

int PostgresDB::addNote(const std::string& name, const std::string& body)
{
    return doAddNote(name, body, {});
}

void PostgresDB::deleteNote(const std::string& name)
{
    ResultWrapper result = connection->exec("DELETE FROM notes WHERE name = $1", name);
    if (!result.valid()) {
        throw std::runtime_error{"Error while deleting note " + name + " " + std::to_string(result.status())};
    }
}

void PostgresDB::deleteNote(int id)
{
    ResultWrapper result = connection->exec("DELETE FROM notes WHERE id = $1", id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while deleting note " + std::to_string(id) + " " +
                                 std::to_string(result.status())};
    }
}

std::string PostgresDB::getNote(const std::string& name)
{
    ResultWrapper result = connection->exec("SELECT body FROM notes WHERE name = $1;", name);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select note " + name + " " + std::to_string(result.status())};
    }
    if (result.empty()) {
        return "";
    }
    return result.getOnlyOne<std::string>();
}

std::string PostgresDB::getNote(int id)
{
    ResultWrapper result = connection->exec("SELECT body FROM notes WHERE id = $1;", id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select note " + std::to_string(id) + " " +
                                 std::to_string(result.status())};
    }
    if (result.empty()) {
        return "";
    }
    return result.getOnlyOne<std::string>();
}

std::pair<std::string, std::string> PostgresDB::getNoteWithName(int id)
{
    // "SELECT name, body FROM notes WHERE id = $1;"
    ResultWrapper result = connection->exec("SELECT name, body FROM notes WHERE id = $1;", id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select note " + std::to_string(id) + " " +
                                 std::to_string(result.status())};
    }
    if (result.empty()) {
        return {"", ""};
    }
    return {result.get<std::string>(0, 0), result.get<std::string>(0, 1)};
}

std::string PostgresDB::getNoteName(int id)
{
    ResultWrapper result = connection->exec("SELECT name FROM notes WHERE id = $1;", id);
    if (!result.valid()) {
        throw std::runtime_error{"Error while select note name " + std::to_string(id) + " " +
                                 std::to_string(result.status())};
    }
    if (result.empty()) {
        return "";
    }
    return result.getOnlyOne<std::string>();
}

int PostgresDB::addNote(const std::string& name, const std::string& body, const std::string& section_name)
{
    int section_id = getOrAddSection(section_name);
    return doAddNote(name, body, section_id);
}

int PostgresDB::addNote(const std::string& name, const std::string& body, int section_id)
{
    return doAddNote(name, body, section_id);
}

int PostgresDB::doAddNote(const std::string& name, const std::string& body, std::optional<int> section_id)
{
    ResultWrapper result =
        connection->exec("INSERT INTO notes (name, body, section_id) VALUES ($1, $2, $3) RETURNING id;", name,
                         body, section_id);
    if (!result.valid() || result.empty()) {
        throw std::runtime_error{"Error while adding note " + name + " " + std::to_string(result.status())};
    }
    return result.getOnlyOne<int>();
}

std::vector<std::pair<int, std::string>> PostgresDB::getAllNoteNames()
{
    ResultWrapper result = connection->exec("SELECT id, name FROM notes;");
    if (!result.valid()) {
        throw std::runtime_error{"Error while select all notes names " + std::to_string(result.status())};
    }
    std::vector<std::pair<int, std::string>> names;
    names.reserve(result.rows());
    for (int i = 0; i < result.rows(); ++i) {
        names.emplace_back(result.get<int>(i, 0), result.get<std::string>(i, 1));
    }
    return names;
}
