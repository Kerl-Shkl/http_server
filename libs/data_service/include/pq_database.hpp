#pragma once

#include "database_interface.hpp"
#include <cassert>
#include <memory>

class PQConnection;

class PostgresDB : public IDataBase
{
public:
    PostgresDB(std::string conn_string);

    PostgresDB(const PostgresDB&) = delete;
    PostgresDB(PostgresDB&&) = default;
    PostgresDB& operator=(const PostgresDB&) = delete;
    PostgresDB& operator=(PostgresDB&&) = delete;
    ~PostgresDB() override;

    int addSection(const std::string& section_name) override;
    void deleteSection(const std::string& section_name) override;
    std::string getSection(int id) override;
    int getOrAddSection(const std::string& section_name) override;
    std::optional<int> getSectionIdByNote(int note_id) override;

    int addNote(const std::string& name, const std::string& body) override;
    void deleteNote(const std::string& name) override;
    void deleteNote(int id) override;

    std::string getNote(const std::string& name) override;
    std::string getNote(int id) override;
    std::pair<std::string, std::string> getNoteWithName(int id) override;
    std::string getNoteName(int id) override;

    int addNote(const std::string& name, const std::string& body, const std::string& section_name) override;
    int addNote(const std::string& name, const std::string& body, int section_id) override;

    std::vector<std::pair<int, std::string>> getAllNoteNames() override;

private:
    int doAddNote(const std::string& name, const std::string& body, std::optional<int> section_id);

    std::unique_ptr<PQConnection> connection;
};
