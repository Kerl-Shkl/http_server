#pragma once

#include "abstract_serialized.hpp"
#include "linux_connection.hpp"
#include "logger.hpp"

class PermissionsController;

class BotCommunicator : public AbstractSerialized
{
public:
    BotCommunicator(PermissionsController& controller);
    ~BotCommunicator() override = default;
    BotCommunicator(const BotCommunicator&) = delete;
    BotCommunicator(BotCommunicator&&) = delete;
    BotCommunicator& operator=(const BotCommunicator&) = delete;
    BotCommunicator& operator=(BotCommunicator&&) = delete;

    void handleIn() override;
    void handleOut() override;

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;

private:
    [[nodiscard]] int openSocket() const;

    PermissionsController& permissions_controller;
    LinuxConnection lcon;
    Logger logger{"BotCommunicator"};
    static constexpr std::string_view socket_path = "/tmp/permissions_bot_socket";
};
