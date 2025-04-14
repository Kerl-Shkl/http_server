#pragma once

#include "bot_messages.hpp"
#include <functional>
#include <unordered_map>

class PermissionsController
{
public:
    using callback_fn = std::function<void(bool)>;

    void askPermission(std::string note_name, RequestOperation op, callback_fn fn);

private:
    std::unordered_map<uuids::uuid, callback_fn> post_actions;
};
