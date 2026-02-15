#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

class IDataBase
{
public:
    virtual ~IDataBase() = default;

    virtual int addSection(const std::string& section_name) = 0;
    virtual void deleteSection(const std::string& section_name) = 0;
    virtual std::string getSection(int id) = 0;
    virtual int getOrAddSection(const std::string& section_name) = 0;
    virtual std::optional<int> getSectionIdByNote(int note_id) = 0;

    virtual int addNote(const std::string& name, const std::string& body) = 0;
    virtual void deleteNote(const std::string& name) = 0;
    virtual void deleteNote(int id) = 0;

    virtual std::string getNote(const std::string& name) = 0;
    virtual std::string getNote(int id) = 0;
    virtual std::pair<std::string, std::string> getNoteWithName(int id) = 0;
    virtual std::string getNoteName(int id) = 0;

    virtual int addNote(const std::string& name, const std::string& body,
                        const std::string& section_name) = 0;
    virtual int addNote(const std::string& name, const std::string& body, int section_id) = 0;

    virtual std::vector<std::pair<int, std::string>> getAllNoteNames() = 0;
};
