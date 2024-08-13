#pragma once

#include <cassert>
#include <utility>

template<typename Connection>
class ConnectionsList
{
    struct Node
    {
        Node(Connection&& c)
        : connection(std::move(c))
        , next(nullptr)
        , prev(nullptr)
        {}
        Connection connection;
        Node *next = nullptr;
        Node *prev = nullptr;
    };

public:
    using Node_ptr = Node *;

    ConnectionsList() = default;

    Connection& getConnection(Node_ptr node)
    {
        return node->connection;
    }

    void pushBack(Connection&& conn)
    {
        Node *new_node = new Node(std::move(conn));
        if (front == nullptr) {
            back = new_node;
            front = new_node;
        }
        else {
            back->next = new_node;
            new_node->prev = back;
            back = new_node;
        }
    }

    void removeNode(Node_ptr node)
    {
        assert(node != nullptr);
        assert(!empty());

        if (back == node && front == node) {
            front = nullptr;
            back = nullptr;
        }
        else if (node == front) {
            front = node->next;
            front->prev = nullptr;
        }
        else if (node == back) {
            back = node->prev;
            back->next = nullptr;
        }
        else {
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }
        delete node;
    }

    bool empty() const noexcept
    {
        return front == nullptr;
    }

    Node_ptr getBackNode()
    {
        return back;
    }

private:
    Node *front = nullptr;
    Node *back = nullptr;
};
