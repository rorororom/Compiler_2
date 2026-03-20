#include "interpreter_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "program.h"

void InterpreterVisitor::visit(Program* node) {
    for (auto& stmt : node->getStatements()) {
        stmt->accept(this);
    }
}

void InterpreterVisitor::visit(DeclareStmt* node) {
    ctx.declare(node->getName());
}

void InterpreterVisitor::visit(AssignStmt* node) {
    node->getExpression()->accept(this);
    int value = lastExpressionValue;
    ctx.assign(node->getName(), value);
}

void InterpreterVisitor::visit(PrintStmt* node) {
    node->getExpression()->accept(this);
    ctx.print(lastExpressionValue);
}

void InterpreterVisitor::visit(IfStmt* node) {
    node->getCondition()->accept(this);
    int condition = lastExpressionValue;
    
    if (condition) {
        for (auto& stmt : node->getThenBranch()) {
            stmt->accept(this);
        }
    } else {
        for (auto& stmt : node->getElseBranch()) {
            stmt->accept(this);
        }
    }
}

void InterpreterVisitor::visit(NumberExpr* node) {
    lastExpressionValue = node->getValue();
}

void InterpreterVisitor::visit(VariableExpr* node) {
    lastExpressionValue = ctx.get(node->getName());
}

void InterpreterVisitor::visit(BinaryExpr* node) {
    node->getLeft()->accept(this);
    int left = lastExpressionValue;
    
    node->getRight()->accept(this);
    int right = lastExpressionValue;
    
    switch (node->getOperator()) {
        case TokenType::PLUS:  lastExpressionValue = left + right; break;
        case TokenType::MINUS: lastExpressionValue = left - right; break;
        case TokenType::MUL:  lastExpressionValue = left * right; break;
        case TokenType::DIV:
            if (right == 0) throw std::runtime_error("Division by zero");
            lastExpressionValue = left / right;
            break;
        default:
            throw std::runtime_error("Unknown operator");
    }
}

void InterpreterVisitor::visit(EqualExpr* node) {
    node->getLeft()->accept(this);
    int left = lastExpressionValue;
    
    node->getRight()->accept(this);
    int right = lastExpressionValue;
    
    lastExpressionValue = (left == right) ? 1 : 0;
}
