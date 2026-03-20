#pragma once

// Forward declarations for all AST node types
class Program;
class DeclareStmt;
class AssignStmt;
class PrintStmt;
class IfStmt;
class NumberExpr;
class VariableExpr;
class BinaryExpr;
class EqualExpr;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(Program* node) = 0;
    virtual void visit(DeclareStmt* node) = 0;
    virtual void visit(AssignStmt* node) = 0;
    virtual void visit(PrintStmt* node) = 0;
    virtual void visit(IfStmt* node) = 0;

    virtual void visit(NumberExpr* node) = 0;
    virtual void visit(VariableExpr* node) = 0;
    virtual void visit(BinaryExpr* node) = 0;
    virtual void visit(EqualExpr* node) = 0;
};