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
        Node *next;
        Node *prev;
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
        if (back != nullptr) {
            back->next = new_node;
        }
        else {
            front = new_node;
        }
        back = new_node;
    }

    void removeNode(Node_ptr node)
    {
        assert(node != nullptr);
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        }
        if (node->next != nullptr) {
            node->next->prev = node->prev;
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
