#include "database.hpp"

DataBase::DataBase(const std::string& connection_string)
: connection(connection_string.c_str())
{
    registerPrepared();
}

int DataBase::addSection(const std::string& section_name)
{
    pqxx::work transaction{connection};
    pqxx::row id_row = transaction.exec_prepared1("addSection", section_name);
    int id = id_row[0].as<int>();
    transaction.commit();
    return id;
}

void DataBase::deleteSection(const std::string& section_name)
{
    pqxx::work transaction{connection};
    transaction.exec_prepared0("deleteSection", section_name);
    transaction.commit();
}

std::string DataBase::getSection(int id)
{
    pqxx::nontransaction action{connection};
    pqxx::result extracted = action.exec_prepared("getSection", id);
    return extractedToString(extracted);
}

int DataBase::addNote(const std::string& name, const std::string& body, int section_id)
{
    return doAddNote(name, body, section_id);
}

int DataBase::addNote(const std::string& name, const std::string& body)
{
    return doAddNote(name, body, std::nullopt);
}

int DataBase::doAddNote(const std::string& name, const std::string& body, std::optional<int> section_id)
{
    pqxx::work transaction{connection};
    pqxx::row id_row = transaction.exec_prepared1("addNote", name, body, section_id);
    int id = id_row[0].as<int>();
    transaction.commit();
    return id;
}

void DataBase::deleteNote(const std::string& name)
{
    pqxx::work transaction{connection};
    transaction.exec_prepared0("deleteNote", name);
    transaction.commit();
}

std::string DataBase::getNote(const std::string& name)
{
    pqxx::nontransaction action{connection};
    pqxx::result extracted = action.exec_prepared("getNoteByName", name);
    return extractedToString(extracted);
}

std::string DataBase::getNote(int id)
{
    pqxx::nontransaction action{connection};
    pqxx::result extracted = action.exec_prepared("getNoteById", id);
    return extractedToString(extracted);
}

std::vector<std::pair<int, std::string>> DataBase::getAllNoteNames()
{
    pqxx::nontransaction action{connection};
    pqxx::result name_rows = action.exec_prepared("getAllNoteNames");
    std::vector<std::pair<int, std::string>> names;
    names.reserve(name_rows.size());
    for (auto row : name_rows) {
        int id = row[0].as<int>();
        auto name = row[1].as<std::string>();
        names.emplace_back(id, std::move(name));
    }
    return names;
}

void DataBase::registerPrepared()
{
    connection.prepare("addSection", "INSERT INTO sections (name) VALUES ($1) RETURNING id");
    connection.prepare("deleteSection", "DELETE FROM sections WHERE name = $1;");
    connection.prepare("getSection", "SELECT name FROM sections WHERE id = $1;");

    connection.prepare("addNote",
                       "INSERT INTO notes (name, body, section_id) VALUES ($1, $2, $3) RETURNING id;");
    connection.prepare("deleteNote", "DELETE FROM notes WHERE name = $1");
    connection.prepare("getNoteByName", "SELECT body FROM notes WHERE name = $1;");
    connection.prepare("getNoteById", "SELECT body FROM notes WHERE id = $1;");
    connection.prepare("getAllNoteNames", "SELECT id, name FROM notes;");
}

[[nodiscard]] std::string DataBase::extractedToString(const pqxx::result& extracted) const
{
    return extracted.empty() ? "" : extracted.front()[0].as<std::string>();
}
