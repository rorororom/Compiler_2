#pragma once
#include "visitor.h"
#include "context.h"

class InterpreterVisitor : public Visitor {
    Context ctx;
    int lastExpressionValue = 0;

public:
    InterpreterVisitor();

    Context& getContext() { return ctx; }
};
