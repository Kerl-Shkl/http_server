#pragma once

#include "linux_connection.hpp"
#include "serialized_interface.hpp"
#include <iostream>

class Connection : public AbstractSerialized
{
public:
    Connection(int socket)
    : lcon(socket)
    {}

    void handleIn() override
    {
        body = lcon.readMessage();
        std::cout << body << std::endl;
    }

    void handleOut() override
    {
        body = lcon.writeMessage(body);
    }

    [[nodiscard]] int getFd() const override
    {
        return lcon.getSocket();
    }
    [[nodiscard]] bool wantIn() const override
    {
        return !closed();
    }
    [[nodiscard]] bool wantOut() const override
    {
        return !body.empty();
    }

    [[nodiscard]] bool closed() const
    {
        return lcon.closed();
    }

private:
    LinuxConnection lcon;
    std::string body;
};
