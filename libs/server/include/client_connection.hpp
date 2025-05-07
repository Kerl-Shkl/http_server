#pragma once

#include "abstract_serialized.hpp"
#include "linux_connection.hpp"
#include "logical_controller.hpp"
#include "request_builder.hpp"
#include <chrono>
#include <string>

class ClientConnection : public AbstractSerialized
{
public:
    ClientConnection(int socket, LogicalController& controller);

    void handleIn() override;
    void handleOut() override;
    std::chrono::system_clock::time_point getWakeup() const noexcept;
    void updateWakeup();

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;
    [[nodiscard]] bool closed() const;

private:
    void processRequest();
    HttpRequest extractRequest();

    using defered_action_t = std::pair<std::function<void()>, std::chrono::system_clock::time_point>;

    LinuxConnection lcon;
    RequestBuilder request_builder;
    std::string response_message;
    LogicalController& controller;
    std::chrono::system_clock::time_point wakeup;
    defered_action_t defered_action;

    static constexpr auto hold_connection_time = std::chrono::minutes(2);
    Logger logger{"ClientConnection"};
};
