#pragma once

#include <memory>

template<typename T>
struct PtrHash
{
    using hash_type = std::hash<std::shared_ptr<T>>;
    using is_transparent = void;

    std::size_t operator()(const std::shared_ptr<T>& con) const
    {
        return hash_type{}(con);
    }

    std::size_t operator()(const void *con) const
    {
        return std::hash<const void *>{}(con);
    }
};

template<typename T>
struct PtrEqualTo
{
    using is_transparent = void;

    bool operator()(const std::shared_ptr<T>& lhp, const std::shared_ptr<T>& rhp) const
    {
        return lhp == rhp;
    }

    bool operator()(const void *lhp, const std::shared_ptr<T>& rhp) const
    {
        return lhp == rhp.get();
    }

    bool operator()(const std::shared_ptr<T>& lhp, const void *rhp) const
    {
        return lhp.get() == rhp;
    }
};

struct StringViewHash
{
    using is_transparent = void;

    std::size_t operator()(const std::string& con) const
    {
        return std::hash<std::string>{}(con);
    }

    std::size_t operator()(const std::string_view con) const
    {
        return std::hash<std::string_view>{}(con);
    }
};

struct StringViewEqualTo
{
    using is_transparent = void;

    bool operator()(const std::string& lhp, const std::string& rhp) const
    {
        return lhp == rhp;
    }

    bool operator()(const std::string_view lhp, const std::string& rhp) const
    {
        return lhp == rhp;
    }

    bool operator()(const std::string& lhp, const std::string_view rhp) const
    {
        return lhp == rhp;
    }
};
