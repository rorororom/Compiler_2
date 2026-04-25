#pragma once
#include "visitor.h"

class Context;

class Node {
public:
    virtual void execute(Context& ctx) = 0;

    virtual void accept(Visitor* visitor) {
        visitor->dispatch(this);
    }

    virtual ~Node() = default;
};
