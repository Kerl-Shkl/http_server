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
    int getOrAddSection(const std::string& section_name);
    std::optional<int> getSectionIdByNote(int note_id);

    int addNote(const std::string& name, const std::string& body);
    void deleteNote(const std::string& name);
    void deleteNote(int id);
    std::string getNote(const std::string& name);

    int addNote(const std::string& name, const std::string& body, const std::string& section_name);
    int addNote(const std::string& name, const std::string& body, int section_id);
    std::string getNote(int id);

    std::vector<std::pair<int, std::string>> getAllNoteNames();

private:
    int doAddNote(const std::string& name, const std::string& body, std::optional<int> section_id);
    [[nodiscard]] std::string extractedToString(const pqxx::result& extracted) const;
    void registerPrepared();

    pqxx::connection connection;
};
