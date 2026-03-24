#pragma once
class Context;

class Node {
public:
    virtual void execute(Context& ctx) = 0;
    virtual ~Node() = default;
};