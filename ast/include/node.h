
#pragma once

class Context;
class Visitor;

class Node {
public:
    virtual void execute(Context& ctx) = 0;
    virtual void accept(Visitor* visitor) = 0;
    virtual ~Node() = default;
};