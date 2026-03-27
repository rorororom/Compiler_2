#pragma once

class Program;
class DeclareStmt;
class AssignStmt;
class PrintStmt;
class IfStmt;
class NumberExpr;
class VariableExpr;
class BinaryExpr;
class EqualExpr;

class ClassDecl;
class FieldDecl;
class MethodDecl;
class ReturnStmt;
class VarDeclStmt;
class NewObjectExpr;
class NewArrayExpr;
class MethodCallExpr;
class FieldAccessExpr;
class ThisExpr;
class ArrayAccessExpr;
class ArrayLengthExpr;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(Program*      node) = 0;
    virtual void visit(DeclareStmt*  node) = 0;
    virtual void visit(AssignStmt*   node) = 0;
    virtual void visit(PrintStmt*    node) = 0;
    virtual void visit(IfStmt*       node) = 0;
    virtual void visit(NumberExpr*   node) = 0;
    virtual void visit(VariableExpr* node) = 0;
    virtual void visit(BinaryExpr*   node) = 0;
    virtual void visit(EqualExpr*    node) = 0;

    virtual void visit(ClassDecl*       node) = 0;
    virtual void visit(FieldDecl*       node) = 0;
    virtual void visit(MethodDecl*      node) = 0;
    virtual void visit(ReturnStmt*      node) = 0;
    virtual void visit(VarDeclStmt*     node) = 0;
    virtual void visit(NewObjectExpr*   node) = 0;
    virtual void visit(NewArrayExpr*    node) = 0;
    virtual void visit(MethodCallExpr*  node) = 0;
    virtual void visit(FieldAccessExpr* node) = 0;
    virtual void visit(ThisExpr*        node) = 0;
    virtual void visit(ArrayAccessExpr* node) = 0;
    virtual void visit(ArrayLengthExpr* node) = 0;
};
