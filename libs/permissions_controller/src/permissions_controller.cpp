#include "permissions_controller.hpp"

void PermissionsController::askPermission(std::string note_name, RequestOperation op, callback_fn fn)
{
    auto uuid = uuids::random_generator{}();
    auto [it, success] = post_actions.emplace(uuid, std::move(fn));
    assert(success);
    BotRequest request{.id = uuid, .op = op, .name = std::move(note_name)};
    // process request
}
