#pragma once

#include "pq_utils.hpp"
#include <cassert>
#include <postgresql/libpq-fe.h>
#include <string>

class PQConnection
{
public:
    PQConnection(std::string con_string)
    : connection_string{std::move(con_string)}
    {}

    PQConnection(PQConnection&&) = default;

    ~PQConnection()
    {
        if (connected()) {
            disconnect();
        }
    }

    PQConnection(const PQConnection&) = delete;
    PQConnection& operator=(const PQConnection&) = delete;
    PQConnection& operator=(PQConnection&&) = delete;

    void connect() noexcept
    {
        connection = PQconnectdb(connection_string.c_str());
        assert(connection != nullptr);
    }

    void disconnect() noexcept
    {
        PQfinish(connection);
    }

    bool connected() const noexcept
    {
        return PQstatus(connection) == CONNECTION_OK;
    }

    void reconnect() noexcept
    {
        PQreset(connection);
    }

    template<typename... Args>
    ResultWrapper exec(const std::string& query, Args&&...args)
    {
        selfCheck();
        return execParams(connection, query, std::forward<Args>(args)...);
    }

    bool transactionReady() const noexcept
    {
        return PQtransactionStatus(connection) == PQTRANS_IDLE;
    }

    void beginTransaction()
    {
        selfCheck();
        ResultWrapper begin_result = exec("BEGIN;");
        if (!begin_result.valid()) {
            throw std::runtime_error{"Error while begin transaction: " +
                                     std::to_string(begin_result.status())};
        }
    }

    void commitTransaction()
    {
        selfCheck();
        ResultWrapper commit_result = exec("COMMIT;");
        if (!commit_result.valid()) {
            throw std::runtime_error{"Error while commit transaction: " +
                                     std::to_string(commit_result.status())};
        }
    }

    void rollbackTransaction()
    {
        selfCheck();
        ResultWrapper rollback_transaction = exec("ROLLBACK;");
        if (!rollback_transaction.valid()) {
            throw std::runtime_error{"Error while rollback transaction: " +
                                     std::to_string(rollback_transaction.status())};
        }
    }

private:
    void selfCheck()
    {
        if (!connected()) {
            reconnect();
            if (!connected()) {
                throw std::runtime_error{"Unable to establish connection to the database"};
            }
        }
    }

    const std::string connection_string;
    PGconn *connection{nullptr};
};
