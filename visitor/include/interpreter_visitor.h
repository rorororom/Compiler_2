#pragma once
#include "visitor.h"
#include "context.h"

class InterpreterVisitor : public Visitor {
    Context ctx;
    int lastExpressionValue = 0;
    
public:
    InterpreterVisitor() = default;
    
    void visit(Program* node) override;
    void visit(DeclareStmt* node) override;
    void visit(AssignStmt* node) override;
    void visit(PrintStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(NumberExpr* node) override;
    void visit(VariableExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(EqualExpr* node) override;
    
    Context& getContext() { return ctx; }
};
