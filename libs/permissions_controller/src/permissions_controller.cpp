#include "permissions_controller.hpp"

PermissionsController::PermissionsController()
: bot_communicator{*this}
{}

void PermissionsController::askPermission(std::string note_name, RequestOperation op, callback_fn fn)
{
    auto uuid = uuids::random_generator{}();
    auto [it, success] = post_actions.emplace(uuid, std::move(fn));
    assert(success);
    logger.log("ask permissions for " + note_name + " with id: ", uuids::to_string(uuid));
    BotRequest request{.id = uuid, .op = op, .name = std::move(note_name)};
    bot_communicator.askRequest(request);
}

void PermissionsController::handleResponse(BotResponse response)
{
    if (auto it = post_actions.find(response.id); it != post_actions.end()) {
        it->second(response.allowed);
        post_actions.erase(it);
        logger.log("Permissions for " + uuids::to_string(response.id) + ": ", response.allowed);
    }
    else {
        logger.log("Unknown permissions for " + uuids::to_string(response.id) + ": ", response.allowed);
    }
}
