#pragma once

#include "database_interface.hpp"
#include "logger.hpp"
#include "postgresql/libpq-fe.h"
#include <cassert>

class PQDatabase : public IDataBase
{
public:
    PQDatabase(std::string conn_string);

    PQDatabase(const PQDatabase&) = default;
    PQDatabase(PQDatabase&&) = delete;
    PQDatabase& operator=(const PQDatabase&) = default;
    PQDatabase& operator=(PQDatabase&&) = delete;
    ~PQDatabase() override = default;

    int addSection(const std::string& section_name) override
    {
        return {};
    }
    void deleteSection(const std::string& section_name) override
    {}
    std::string getSection(int id) override
    {
        return {};
    }
    int getOrAddSection(const std::string& section_name) override
    {
        return {};
    }
    std::optional<int> getSectionIdByNote(int note_id) override
    {
        return {};
    }

    int addNote(const std::string& name, const std::string& body) override;
    void deleteNote(const std::string& name) override
    {}
    void deleteNote(int id) override
    {}

    std::string getNote(const std::string& name) override
    {
        return {};
    }
    std::string getNote(int id) override;
    std::pair<std::string, std::string> getNoteWithName(int id) override
    {
        return {};
    }
    std::string getNoteName(int id) override
    {
        return {};
    }

    int addNote(const std::string& name, const std::string& body, const std::string& section_name) override
    {
        return {};
    }
    int addNote(const std::string& name, const std::string& body, int section_id) override
    {
        return {};
    }

    std::vector<std::pair<int, std::string>> getAllNoteNames() override
    {
        return {};
    }

private:
    void connect() noexcept;
    bool isConnected() const noexcept;
    template<typename... Args>
    PGresult *execParams(const std::string& query, bool bin_result = false, Args&&...args);

    std::string connection_string;
    PGconn *connection{nullptr};

    Logger logger{"pq_db"};
};
