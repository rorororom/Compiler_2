#include "expressions.h"
#include "visitor.h"
#include "context.h"
#include <stdexcept>

int VariableExpr::evaluate(Context& ctx) {
    return ctx.get(name);
}

void NumberExpr::accept(Visitor* visitor) {
    visitor->visit(this);
}

void VariableExpr::accept(Visitor* visitor) {
    visitor->visit(this);
}

int BinaryExpr::evaluate(Context& ctx) {
    int leftVal = left->evaluate(ctx);
    int rightVal = right->evaluate(ctx);
    
    switch (op) {
        case TokenType::PLUS:  return leftVal + rightVal;
        case TokenType::MINUS: return leftVal - rightVal;
        case TokenType::MUL:  return leftVal * rightVal;
        case TokenType::DIV:
            if (rightVal == 0)
                throw std::runtime_error("Division by zero");
            return leftVal / rightVal;
        default:
            throw std::runtime_error("Unknown operator");
    }
}

void BinaryExpr::accept(Visitor* visitor) {
    visitor->visit(this);
}

int EqualExpr::evaluate(Context& ctx) {
    return left->evaluate(ctx) == right->evaluate(ctx);
}

void EqualExpr::accept(Visitor* visitor) {
    visitor->visit(this);
}