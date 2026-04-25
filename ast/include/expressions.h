#pragma once
#include "expression.h"
#include "token.h"
#include <string>
#include <memory>

class NumberExpr : public Expression {
    int value;
public:
    explicit NumberExpr(int v) : value(v) {}

    int evaluate(Context& ctx) override { return value; }

    int getValue() const { return value; }
};

class VariableExpr : public Expression {
    std::string name;
public:
    explicit VariableExpr(std::string n) : name(std::move(n)) {}

    int evaluate(Context& ctx) override;

    const std::string& getName() const { return name; }
};

class BinaryExpr : public Expression {
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;

public:
    BinaryExpr(std::unique_ptr<Expression> l, TokenType op, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(op), right(std::move(r)) {}

    int evaluate(Context& ctx) override;

    Expression* getLeft()  { return left.get(); }
    Expression* getRight() { return right.get(); }
    TokenType   getOperator() const { return op; }
};

class EqualExpr : public Expression {
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

public:
    EqualExpr(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}

    int evaluate(Context& ctx) override;

    Expression* getLeft()  { return left.get(); }
    Expression* getRight() { return right.get(); }
};
