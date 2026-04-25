#include "interpreter_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "program.h"
#include <stdexcept>

InterpreterVisitor::InterpreterVisitor() {
    registerHandler<Program>([this](Program* node) {
        for (auto& stmt : node->getStatements())
            stmt->accept(this);
    });

    registerHandler<DeclareStmt>([this](DeclareStmt* node) {
        ctx.declare(node->getName());
    });

    registerHandler<AssignStmt>([this](AssignStmt* node) {
        node->getExpression()->accept(this);
        ctx.assign(node->getName(), lastExpressionValue);
    });

    registerHandler<PrintStmt>([this](PrintStmt* node) {
        node->getExpression()->accept(this);
        ctx.print(lastExpressionValue);
    });

    registerHandler<IfStmt>([this](IfStmt* node) {
        node->getCondition()->accept(this);
        int condition = lastExpressionValue;

        if (condition) {
            for (auto& stmt : node->getThenBranch())
                stmt->accept(this);
        } else {
            for (auto& stmt : node->getElseBranch())
                stmt->accept(this);
        }
    });

    registerHandler<NumberExpr>([this](NumberExpr* node) {
        lastExpressionValue = node->getValue();
    });

    registerHandler<VariableExpr>([this](VariableExpr* node) {
        lastExpressionValue = ctx.get(node->getName());
    });

    registerHandler<BinaryExpr>([this](BinaryExpr* node) {
        node->getLeft()->accept(this);
        int left = lastExpressionValue;

        node->getRight()->accept(this);
        int right = lastExpressionValue;

        switch (node->getOperator()) {
            case TokenType::PLUS:  lastExpressionValue = left + right; break;
            case TokenType::MINUS: lastExpressionValue = left - right; break;
            case TokenType::MUL:   lastExpressionValue = left * right; break;
            case TokenType::DIV:
                if (right == 0) throw std::runtime_error("Division by zero");
                lastExpressionValue = left / right;
                break;
            default:
                throw std::runtime_error("Unknown operator");
        }
    });

    registerHandler<EqualExpr>([this](EqualExpr* node) {
        node->getLeft()->accept(this);
        int left = lastExpressionValue;

        node->getRight()->accept(this);
        int right = lastExpressionValue;

        lastExpressionValue = (left == right) ? 1 : 0;
    });

}
