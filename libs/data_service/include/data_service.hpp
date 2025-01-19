#pragma once

#include <string>
#include <string_view>

class DataService
{
public:
    [[nodiscard]] std::string getNote(const std::string_view note_name) const;
    [[nodiscard]] std::string getNote(int note_id) const;
};
