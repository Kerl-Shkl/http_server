#include "pq_database.hpp"
#include <netinet/in.h>

namespace {

template<typename T>
struct ParamWrapper;

template<>
struct ParamWrapper<std::string>
{
    ParamWrapper(std::string& s)
    : ref_s{s}
    {}

    ParamWrapper(std::string&& s)  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
    : ref_s{s}
    {}

    const char *pointer()
    {
        return ref_s.c_str();
    }

    int size()
    {
        return 0;
    }

    int format()
    {
        return 0;  // text
    }

private:
    std::string& ref_s;
};

template<>
struct ParamWrapper<int>
{
    ParamWrapper(int i)
    : network_value{static_cast<int>(htonl(i))}
    {}

    const char *pointer()
    {
        return reinterpret_cast<char *>(&network_value);
    }

    int size()
    {
        return sizeof(int);
    }

    int format()
    {
        return 1;  // binary
    }

private:
    int network_value;
};

template<typename Tuple, typename Func, int n>
struct Filler
{
    template<typename Arr>
    static void fill(Arr arr, Func f, Tuple& t)
    {
        Filler<Tuple, Func, n - 1>::fill(arr, f, t);
        arr[n - 1] = f(std::get<n - 1>(t));
    }
};

template<typename Tuple, typename Func>
struct Filler<Tuple, Func, 1>
{
    template<typename Arr>
    static void fill(Arr arr, Func f, Tuple& t)
    {
        arr[0] = f(std::get<0>(t));
    }
};

template<typename ArrType, typename... Args, typename Func>
void fillArray(ArrType arr, Func f, std::tuple<Args...>& t)
{
    if constexpr (sizeof...(Args) > 0) {
        Filler<decltype(t), Func, sizeof...(Args)>::fill(arr, f, t);
    }
}

}  // namespace

template<typename... Args>
PGresult *PQDatabase::execParams(const std::string& query, bool bin_result, Args&&...args)
{
    std::tuple<ParamWrapper<std::remove_cvref_t<Args>...>> tup{std::forward<Args>(args)...};
    const char *params_ptr[sizeof...(Args)];
    int params_size[sizeof...(Args)];
    int params_format[sizeof...(Args)];
    fillArray(params_ptr, [](auto& v) { return v.pointer(); }, tup);
    fillArray(params_size, [](auto& v) { return v.size(); }, tup);
    fillArray(params_format, [](auto& v) { return v.format(); }, tup);
    return PQexecParams(connection, query.c_str(), sizeof...(Args), nullptr, params_ptr, params_size,
                        params_format, bin_result ? 1 : 0);
}

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

// std::string PQDatabase::getNote(int id)
// {
//     id = htonl(id);
//     const char *id_param[1] = {reinterpret_cast<char *>(&id)};
//     int id_length[1] = {sizeof(id)};
//     int id_format[1] = {1};  // binary data
//     PGresult *res = PQexecParams(connection, "SELECT body FROM notes WHERE id = $1::integer;", 1, nullptr,
//                                  id_param, id_length, id_format, 0 /* result in text format */);
//
//     if (PQresultStatus(res) != PGRES_TUPLES_OK) {
//         logger.log("getNote failed: ", PQerrorMessage(connection));
//         PQclear(res);
//     }
//     assert(PQntuples(res) == 1);
//     assert(PQnfields(res) == 1);
//     std::string note_body{PQgetvalue(res, 0, 0)};
//     PQclear(res);
//
//     return note_body;
// }

std::string PQDatabase::getNote(int id)
{
    PGresult *res = execParams("SELECT body FROM notes WHERE id = $1::integer;", false, id);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        logger.log("getNote failed: ", PQerrorMessage(connection));
        PQclear(res);
    }
    assert(PQntuples(res) == 1);
    assert(PQnfields(res) == 1);
    std::string note_body{PQgetvalue(res, 0, 0)};
    PQclear(res);

    return note_body;
}

int PQDatabase::addNote(const std::string& name, const std::string& body)
{
    return {};
}
