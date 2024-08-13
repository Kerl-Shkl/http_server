#include "connections_list.hpp"
#include <gtest/gtest.h>

TEST(ConnectionsList, EmptyIsEmpty)
{
    ConnectionsList<int> list;
    EXPECT_TRUE(list.empty());
}

TEST(ConnectionsList, PushThenGetBackNodeSuccess)
{
    int a = 42;
    int b = a;
    ConnectionsList<int> list;
    list.pushBack(std::move(b));
    auto *node = list.getBackNode();
    EXPECT_EQ(a, list.getConnection(node));
}

TEST(ConnectionsList, RemoveNodeSuccess)
{
    int a = 42;
    int b = a;
    ConnectionsList<int> list;
    list.pushBack(std::move(b));
    auto *node = list.getBackNode();
    list.removeNode(node);
    EXPECT_TRUE(list.empty());
}

TEST(ConnectionsList, FewElementsList)
{
    std::vector<int> vec{1, 2, 3};
    ConnectionsList<int> list;
    for (auto i : vec) {
        list.pushBack(std::move(i));
    }
    for (auto rit = vec.rbegin(); rit != vec.rend(); ++rit) {
        auto *node = list.getBackNode();
        auto& element = list.getConnection(node);
        ASSERT_EQ(element, *rit);
        list.removeNode(node);
    }
    EXPECT_TRUE(list.empty());
}
