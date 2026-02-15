#pragma once

#include "database_interface.hpp"
#include <optional>
#include <pqxx/pqxx>
#include <string>

class DataBase : public IDataBase
{
public:
    DataBase(const std::string& connection_string);
    int addSection(const std::string& section_name) override;
    void deleteSection(const std::string& section_name) override;
    std::string getSection(int id) override;
    int getOrAddSection(const std::string& section_name) override;
    std::optional<int> getSectionIdByNote(int note_id) override;

    int addNote(const std::string& name, const std::string& body) override;
    void deleteNote(const std::string& name) override;
    void deleteNote(int id) override;
    std::string getNote(const std::string& name) override;

    int addNote(const std::string& name, const std::string& body, const std::string& section_name) override;
    int addNote(const std::string& name, const std::string& body, int section_id) override;
    std::string getNote(int id) override;
    std::pair<std::string, std::string> getNoteWithName(int id) override;
    std::string getNoteName(int id) override;

    std::vector<std::pair<int, std::string>> getAllNoteNames() override;

private:
    int doAddNote(const std::string& name, const std::string& body, std::optional<int> section_id);
    [[nodiscard]] std::string extractedToString(const pqxx::result& extracted) const;
    void registerPrepared();

    pqxx::connection connection;
};
