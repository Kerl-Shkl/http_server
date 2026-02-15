#include "pq_database.hpp"
#include <netinet/in.h>

namespace {

template<typename T>
struct TypeFix
{
    using type = T&;
};
template<typename T>
struct TypeFix<T&&>
{
    using type = T;
};

template<>
struct TypeFix<int>
{
    using type = int;
};

template<typename... Args>
using fixed_tuple = std::tuple<typename TypeFix<Args>::type...>;

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

char *toParamPointer(int& i)
{
    i = htonl(i);
    return reinterpret_cast<char *>(&i);
}

const char *toParamPointer(const std::string& str)
{
    return str.c_str();
}

size_t toParamSize(int& /*unused*/)
{
    return sizeof(int);
}

size_t toParamSize(std::string& s)
{
    return s.size();  // should be unused
}

int toParamFormat(int& /*unused*/)
{
    return 1;  // binary
}

int toParamFormat(std::string& /*unused*/)
{
    return 0;  // text
}

}  // namespace

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

template<typename... Args>
PGresult *PQDatabase::execParams(const std::string& query, bool bin_result, Args&&...args)
{
    fixed_tuple<Args&&...> tup{std::forward<Args>(args)...};
    char *params_ptr[sizeof...(Args)];
    int params_size[sizeof...(Args)];
    int params_format[sizeof...(Args)];
    fillArray(params_ptr, [](auto& v) { return toParamPointer(v); }, tup);
    fillArray(params_size, [](auto& v) { return toParamSize(v); }, tup);
    fillArray(params_format, [](auto& v) { return toParamFormat(v); }, tup);
    return PQexecParams(connection, query.c_str(), sizeof...(Args), nullptr, params_ptr, params_size,
                        params_format, bin_result ? 1 : 0);
}

int PQDatabase::addNote(const std::string& name, const std::string& body)
{
    return {};
}
