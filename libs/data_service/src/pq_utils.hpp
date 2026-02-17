#pragma once

#include <cassert>
#include <netinet/in.h>
#include <postgresql/libpq-fe.h>
#include <stdexcept>
#include <string>

namespace detail {
enum class ParamFormat : uint8_t
{
    text = 0,
    binary = 1
};

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
        return static_cast<int>(ParamFormat::text);
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
        return static_cast<int>(ParamFormat::binary);
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
}  // namespace detail

class ResultWrapper
{
public:
    ResultWrapper(PGresult *res)
    : result{res}
    {}
    ResultWrapper(ResultWrapper&& rhp) noexcept
    : result{rhp.result}
    {
        rhp.result = nullptr;
    }
    ResultWrapper& operator=(ResultWrapper&& rhp) noexcept
    {
        result = rhp.result;
        rhp.result = nullptr;
        return *this;
    }
    ~ResultWrapper()
    {
        if (result != nullptr) {
            PQclear(result);
        }
    }
    ResultWrapper(const ResultWrapper&) = delete;
    ResultWrapper& operator=(const ResultWrapper&) = delete;

    int rows() const
    {
        return PQntuples(result);
    }

    int columns() const
    {
        return PQnfields(result);
    }

    bool valid()
    {
        auto status = PQresultStatus(result);
        return status == PGRES_COMMAND_OK;
    }

    template<typename T>
    T get(int row, int column);

    template<typename T>
    T getOnlyOne()
    {
        return get<T>(0, 0);
    }

private:
    PGresult *result;
};

template<>
inline std::string ResultWrapper::get<std::string>(int row, int column)
{
    char *value = PQgetvalue(result, row, column);
    int len = PQgetlength(result, row, column);
    if (value == nullptr) {
        throw std::runtime_error("PQDB error. Try to get unavailable value");
    }
    return std::string{value, static_cast<size_t>(len)};
}

template<>
inline int ResultWrapper::get<int>(int row, int column)
{
    char *value = PQgetvalue(result, row, column);
    int len = PQgetlength(result, row, column);
    if (value == nullptr) {
        throw std::runtime_error("PQDB error. Try to get unavailable value");
    }

    switch (static_cast<detail::ParamFormat>(PQfformat(result, column))) {
    case detail::ParamFormat::binary: {
        int v = *reinterpret_cast<int *>(value);
        v = ntohl(v);
        return v;
    }
    case detail::ParamFormat::text: {
        std::string s{value, static_cast<size_t>(len)};
        int res = std::stoi(s);
        return res;
    }
    default:
        assert(false);
    }
    __builtin_unreachable();
}

template<typename... Args>
ResultWrapper execParams(PGconn *connection, const std::string& query, Args&&...args)
{
    std::tuple<detail::ParamWrapper<std::remove_cvref_t<Args>...>> tup{std::forward<Args>(args)...};
    const char *params_ptr[sizeof...(Args)];
    int params_size[sizeof...(Args)];
    int params_format[sizeof...(Args)];
    detail::fillArray(params_ptr, [](auto& v) { return v.pointer(); }, tup);
    detail::fillArray(params_size, [](auto& v) { return v.size(); }, tup);
    detail::fillArray(params_format, [](auto& v) { return v.format(); }, tup);
    PGresult *res = PQexecParams(connection, query.c_str(), sizeof...(Args), nullptr, params_ptr, params_size,
                                 params_format, 1);
    return ResultWrapper{res};
}
