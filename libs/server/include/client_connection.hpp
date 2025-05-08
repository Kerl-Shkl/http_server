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

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;
    [[nodiscard]] bool closed() const;

private:
    void processRequest();
    HttpRequest extractRequest();

    LinuxConnection lcon;
    RequestBuilder request_builder;
    std::string response_message;
    LogicalController& controller;
    std::chrono::system_clock::time_point wakeup;
    Logger logger{"ClientConnection"};
};
