#pragma once
#include "node.h"
#include "expression.h"
#include <string>
#include <vector>
#include <memory>

class DeclareStmt : public Node {
    std::string name;
public:
    explicit DeclareStmt(std::string n) : name(std::move(n)) {}
    
    void execute(Context& ctx) override;
    void accept(Visitor* visitor) override;
    
    const std::string& getName() const { return name; }
};

class AssignStmt : public Node {
    std::string name;
    std::unique_ptr<Expression> expr;
public:
    AssignStmt(std::string n, std::unique_ptr<Expression> e)
        : name(std::move(n)), expr(std::move(e)) {}
    
    void execute(Context& ctx) override;
    void accept(Visitor* visitor) override;
    
    const std::string& getName() const { return name; }
    Expression* getExpression() { return expr.get(); }
};

class PrintStmt : public Node {
    std::unique_ptr<Expression> expr;
public:
    explicit PrintStmt(std::unique_ptr<Expression> e)
        : expr(std::move(e)) {}
    
    void execute(Context& ctx) override;
    void accept(Visitor* visitor) override;
    
    Expression* getExpression() { return expr.get(); }
};

class IfStmt : public Node {
    std::unique_ptr<Expression> cond;
    std::vector<std::unique_ptr<Node>> thenBranch;
    std::vector<std::unique_ptr<Node>> elseBranch;
    
public:
    explicit IfStmt(std::unique_ptr<Expression> c) : cond(std::move(c)) {}
    
    void execute(Context& ctx) override;
    void accept(Visitor* visitor) override;
    
    Expression* getCondition() { return cond.get(); }
    std::vector<std::unique_ptr<Node>>& getThenBranch() { return thenBranch; }
    std::vector<std::unique_ptr<Node>>& getElseBranch() { return elseBranch; }
    
    void addThenStatement(std::unique_ptr<Node> stmt) {
        thenBranch.push_back(std::move(stmt));
    }
    
    void addElseStatement(std::unique_ptr<Node> stmt) {
        elseBranch.push_back(std::move(stmt));
    }
};