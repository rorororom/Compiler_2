#pragma once
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

class Node;

class Visitor {
public:
    virtual ~Visitor() = default;

    template<typename T>
    void registerHandler(std::function<void(T*)> fn) {
        table_[typeid(T)] = [fn](Node* node) {
            fn(static_cast<T*>(node));
        };
    }

    void dispatch(Node* node);

protected:
    std::unordered_map<std::type_index, std::function<void(Node*)>> table_;
};
