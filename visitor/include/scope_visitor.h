#pragma once
#include "visitor.h"
#include "symbol_table.h"
#include "scope_tree.h"
#include "oop_nodes.h"
#include "expressions.h"
#include "statements.h"
#include "program.h"
#include <string>
#include <stdexcept>

class ScopeVisitor : public Visitor {
public:
    ScopeVisitor();

    void collectClasses(Program* program);

    const SymbolTable& getSymbolTable() const { return symTable_; }
    SymbolTable&       getSymbolTable()       { return symTable_; }

    const ScopeTree&   getScopeTree()   const { return scopeTree_; }
    ScopeTree&         getScopeTree()         { return scopeTree_; }

private:
    SymbolTable symTable_;
    ScopeTree   scopeTree_;

    std::string currentClass_;
    std::string currentMethod_;

    void declareVar(const std::string& name, const TypeInfo& type,
                    VariableSymbol::StorageKind storage = VariableSymbol::StorageKind::LOCAL);

    const VariableSymbol* resolveVar(const std::string& name);

    TypeInfo resolveType(const TypeInfo& ann) const;
};
