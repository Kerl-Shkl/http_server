#pragma once

#include <pqxx/pqxx>
#include <string>

class DataBase
{
public:
    DataBase(const std::string& connection_string);
    int addSection(const std::string& section_name);
    void deleteSection(const std::string& section_name);
    std::string getSection(int id);

    int addNote(const std::string& name, const std::string& body);
    void deleteNote(const std::string& name);
    std::string getNote(const std::string& name);

    int addNote(const std::string& name, const std::string& body, int section_id);
    std::string getNote(int id);

    std::vector<std::string> getAllNoteNames();

private:
    int addNote(const std::string& name, const std::string& body, std::optional<int> section_id);
    [[nodiscard]] std::string extractedToString(const pqxx::result& extracted) const;
    void registerPrepared();

    pqxx::connection connection;
};
