#pragma once

#include "bot_communicator.hpp"
#include "bot_messages.hpp"
#include "poller.hpp"
#include <functional>
#include <unordered_map>

class PermissionsController
{
public:
    using callback_fn = std::function<void(bool)>;
    PermissionsController();
    void askPermission(std::string note_name, RequestOperation op, callback_fn fn);
    BotCommunicator& getCommunicator();
    void startBotCommunication();

private:
    void handleResponse(BotResponse response);
    [[nodiscard]] uuids::uuid getRandomId() const;

    friend class BotCommunicator;
    BotCommunicator bot_communicator;
    Logger logger{"PermissionsController"};
    Poller *poller{nullptr};
    std::unordered_map<uuids::uuid, callback_fn> post_actions;
};
