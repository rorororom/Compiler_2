#pragma once
#include "visitor.h"
#include "context.h"

class InterpreterVisitor : public Visitor {
    Context ctx;
    int lastExpressionValue = 0;

public:
    InterpreterVisitor() = default;

    void visit(Program*      node) override;
    void visit(DeclareStmt*  node) override;
    void visit(AssignStmt*   node) override;
    void visit(PrintStmt*    node) override;
    void visit(IfStmt*       node) override;
    void visit(NumberExpr*   node) override;
    void visit(VariableExpr* node) override;
    void visit(BinaryExpr*   node) override;
    void visit(EqualExpr*    node) override;

    void visit(ClassDecl*       node) override {}
    void visit(FieldDecl*       node) override {}
    void visit(MethodDecl*      node) override {}
    void visit(ReturnStmt*      node) override {}
    void visit(VarDeclStmt*     node) override {}
    void visit(NewObjectExpr*   node) override {}
    void visit(NewArrayExpr*    node) override {}
    void visit(MethodCallExpr*  node) override {}
    void visit(FieldAccessExpr* node) override {}
    void visit(ThisExpr*        node) override {}
    void visit(ArrayAccessExpr* node) override {}
    void visit(ArrayLengthExpr* node) override {}

    Context& getContext() { return ctx; }
};
