#pragma once

#include <cassert>
#include <netinet/in.h>
#include <optional>
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

    const char *pointer() const noexcept
    {
        return ref_s.c_str();
    }

    int size() const noexcept
    {
        return 0;
    }

    int format() const noexcept
    {
        return static_cast<int>(ParamFormat::text);
    }

private:
    std::string& ref_s;
};

template<typename T> concept cstr_concept = std::is_convertible_v<T, const char *>;
template<cstr_concept T>
struct ParamWrapper<T>
{
    ParamWrapper(const char *s)
    : str{s}
    {}

    const char *pointer() const noexcept
    {
        return str;
    }

    int size() const noexcept
    {
        return 0;
    }

    int format() const noexcept
    {
        return static_cast<int>(ParamFormat::text);
    }

private:
    const char *str;
};

template<>
struct ParamWrapper<int>
{
    ParamWrapper(int i)
    : network_value{static_cast<int>(htonl(i))}
    {}

    const char *pointer() const noexcept
    {
        return reinterpret_cast<const char *>(&network_value);
    }

    int size() const noexcept
    {
        return sizeof(int);
    }

    int format() const noexcept
    {
        return static_cast<int>(ParamFormat::binary);
    }

private:
    int network_value;
};

template<typename T>
struct ParamWrapper<std::optional<T>>
{
    ParamWrapper(std::optional<T>& v)
    {
        if (v.has_value()) {
            opt.emplace(v.value());
        }
    }

    ParamWrapper(std::optional<T>&& v)  // NOLINT Rvalue reference parameter 'v' is never moved
    {
        if (v.has_value()) {
            opt.emplace(std::move(v.value()));
        }
    }

    const char *pointer() const noexcept
    {
        return opt.has_value() ? opt->pointer() : nullptr;
    }

    int size() const noexcept
    {
        return opt.has_value() ? opt->size() : 0;
    }

    int format() const noexcept
    {
        return opt.has_value() ? opt->format() : static_cast<int>(ParamFormat::binary);
    }

private:
    std::optional<ParamWrapper<T>> opt;
};

template<>
struct ParamWrapper<std::nullopt_t>
{
    ParamWrapper(const std::nullopt_t& /*unused*/)
    {}
    const char *pointer() const noexcept
    {
        return nullptr;
    }
    int size() const noexcept
    {
        return 0;
    }
    int format() const noexcept
    {
        return static_cast<int>(ParamFormat::binary);
    }
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

    int rows() const noexcept
    {
        return PQntuples(result);
    }

    int columns() const noexcept
    {
        return PQnfields(result);
    }

    bool empty() const noexcept
    {
        return rows() == 0;
    }

    bool valid() const noexcept
    {
        auto status = PQresultStatus(result);
        return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK;
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
    PGresult *res{nullptr};
    if constexpr (sizeof...(Args) > 0) {
        std::tuple<detail::ParamWrapper<std::remove_cvref_t<Args>>...> tup{std::forward<Args>(args)...};
        const char *params_ptr[sizeof...(Args)];
        int params_size[sizeof...(Args)];
        int params_format[sizeof...(Args)];
        detail::fillArray(params_ptr, [](auto& v) { return v.pointer(); }, tup);
        detail::fillArray(params_size, [](auto& v) { return v.size(); }, tup);
        detail::fillArray(params_format, [](auto& v) { return v.format(); }, tup);
        res = PQexecParams(connection, query.c_str(), sizeof...(Args), nullptr, params_ptr, params_size,
                           params_format, static_cast<int>(detail::ParamFormat::binary));
    }
    else {
        res = PQexecParams(connection, query.c_str(), 0, nullptr, nullptr, nullptr, nullptr,
                           static_cast<int>(detail::ParamFormat::binary));
    }
    return ResultWrapper{res};
}
