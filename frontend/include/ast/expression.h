#pragma once
#include "node.h"

class Expression : public Node {
public:
    virtual int evaluate(Context& ctx) = 0;
    void execute(Context& ctx) override {}
};