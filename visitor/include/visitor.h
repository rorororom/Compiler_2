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

/**
 * Abstract base class for all visitors.
 * Implements the Visitor pattern for AST traversal.
 */
class Visitor {
public:
    virtual ~Visitor() = default;
    
    // Visit methods for statements
    virtual void visit(Program* node) = 0;
    virtual void visit(DeclareStmt* node) = 0;
    virtual void visit(AssignStmt* node) = 0;
    virtual void visit(PrintStmt* node) = 0;
    virtual void visit(IfStmt* node) = 0;
    
    // Visit methods for expressions
    virtual void visit(NumberExpr* node) = 0;
    virtual void visit(VariableExpr* node) = 0;
    virtual void visit(BinaryExpr* node) = 0;
    virtual void visit(EqualExpr* node) = 0;
};