#pragma once
#include "visitor.h"
#include "symbol_table.h"
#include "scope_tree.h"
#include "oop_nodes.h"
#include <string>
#include <vector>
#include <stdexcept>

class ScopeVisitor : public Visitor {
public:
    ScopeVisitor();

    void collectClasses(Program* program);

    const SymbolTable& getSymbolTable() const { return symTable_; }
    SymbolTable&       getSymbolTable()       { return symTable_; }

    const ScopeTree&   getScopeTree()   const { return scopeTree_; }
    ScopeTree&         getScopeTree()         { return scopeTree_; }

    void visit(Program*      node) override;
    void visit(DeclareStmt*  node) override;
    void visit(AssignStmt*   node) override;
    void visit(PrintStmt*    node) override;
    void visit(IfStmt*       node) override;
    void visit(NumberExpr*   node) override;
    void visit(VariableExpr* node) override;
    void visit(BinaryExpr*   node) override;
    void visit(EqualExpr*    node) override;

    void visit(ClassDecl*       node) override;
    void visit(FieldDecl*       node) override;
    void visit(MethodDecl*      node) override;
    void visit(ReturnStmt*      node) override;
    void visit(VarDeclStmt*     node) override;
    void visit(NewObjectExpr*   node) override;
    void visit(NewArrayExpr*    node) override;
    void visit(MethodCallExpr*  node) override;
    void visit(FieldAccessExpr* node) override;
    void visit(ThisExpr*        node) override;
    void visit(ArrayAccessExpr* node) override;
    void visit(ArrayLengthExpr* node) override;

private:
    SymbolTable symTable_;
    ScopeTree   scopeTree_;

    std::string currentClass_;
    std::string currentMethod_;

    void declareVar(const std::string& name, const TypeInfo& type,
                    VariableSymbol::StorageKind storage = VariableSymbol::StorageKind::LOCAL);

    const VariableSymbol* resolveVar(const std::string& name);

    TypeInfo resolveType(const TypeAnnotation& ann) const;
};
