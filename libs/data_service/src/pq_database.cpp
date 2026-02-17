#include "pq_database.hpp"
#include "pq_utils.hpp"
#include <netinet/in.h>

PQDatabase::PQDatabase(std::string conn_string)
: connection_string{std::move(conn_string)}
{
    connect();
}

void PQDatabase::connect() noexcept
{
    connection = PQconnectdb(connection_string.c_str());
    assert(connection != nullptr);
}

bool PQDatabase::isConnected() const noexcept
{
    return connection != nullptr && PQstatus(connection) == CONNECTION_OK;
}

std::string PQDatabase::getNote(int id)
{
    ResultWrapper res = execParams(connection, "SELECT body FROM notes WHERE id = $1::integer;", id);

    if (!res.valid()) {
        logger.log("getNote failed: ", PQerrorMessage(connection));
        // TODO add check connection
        return {};
    }
    return res.getOnlyOne<std::string>();
}

int PQDatabase::addNote(const std::string& name, const std::string& body)
{
    return {};
}
