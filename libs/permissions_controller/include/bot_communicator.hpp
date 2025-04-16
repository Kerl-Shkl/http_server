#pragma once

#include "abstract_serialized.hpp"
#include "bot_messages.hpp"
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

    void askRequest(const BotRequest& request);

    void handleIn() override;
    void handleOut() override;

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;

private:
    [[nodiscard]] int openSocket() const;

    [[nodiscard]] std::vector<uint8_t> serializeRequest(const BotRequest& request) const;
    [[nodiscard]] BotResponse parseResponse(const std::span<uint8_t> buffer) const;

    std::vector<uint8_t> write_buffer;
    std::vector<uint8_t> read_buffer;
    PermissionsController& permissions_controller;
    LinuxConnection lcon;
    Logger logger{"BotCommunicator"};
    static constexpr std::string_view socket_path = "/tmp/permissions_bot_socket";
};
