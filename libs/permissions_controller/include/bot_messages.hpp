#pragma once

#include <uuid.h>

enum class RequestOperation : uint8_t
{
    add,
    remove
};

struct BotRequest
{
    uuids::uuid id;
    RequestOperation op{RequestOperation::add};
    std::string name;
};

struct BotResponse
{
    uuids::uuid id;
    bool allowed{false};
};
