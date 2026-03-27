#include "scope_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
#include "program.h"
#include <iostream>
#include <stdexcept>

ScopeVisitor::ScopeVisitor() = default;

TypeInfo ScopeVisitor::resolveType(const TypeAnnotation& ann) const {
    if (ann.typeName == "int" || ann.typeName == "void") {
        return ann.toTypeInfo();
    }
    if (!symTable_.findClass(ann.typeName)) {
        throw std::runtime_error(
            "SemanticError: unknown type '" + ann.typeName + "'");
    }
    return ann.toTypeInfo();
}

void ScopeVisitor::declareVar(const std::string& name,
                               const TypeInfo& type,
                               VariableSymbol::StorageKind storage) {
    VariableSymbol sym(name, type, storage);
    scopeTree_.declare(sym, true);
}

const VariableSymbol* ScopeVisitor::resolveVar(const std::string& name) {
    return scopeTree_.lookup(name);
}

void ScopeVisitor::collectClasses(Program* program) {
    for (auto& stmt : program->getStatements()) {
        if (auto* cls = dynamic_cast<ClassDecl*>(stmt.get())) {
            ClassSymbol classSym(cls->getName());

            for (auto& fld : cls->getFields()) {
                TypeInfo ftype = resolveType(fld->getType());
                VariableSymbol vsym(fld->getName(), ftype,
                                    VariableSymbol::StorageKind::FIELD);
                classSym.addField(vsym);
            }

            for (auto& mth : cls->getMethods()) {
                TypeInfo retType = resolveType(mth->getReturnType());
                MethodSymbol msym(mth->getName(), retType, cls->getName());

                for (auto& param : mth->getParams()) {
                    TypeInfo ptype = resolveType(param.type);
                    VariableSymbol psym(param.name, ptype,
                                        VariableSymbol::StorageKind::PARAM);
                    msym.addParam(psym);
                }
                classSym.addMethod(msym);
            }

            classSym.ensureConstructor();
            symTable_.addClass(std::move(classSym));
        }
    }
}

void ScopeVisitor::visit(Program* node) {
    for (auto& stmt : node->getStatements()) {
        stmt->accept(this);
    }
}

void ScopeVisitor::visit(DeclareStmt* node) {
    declareVar(node->getName(), TypeInfo::makeInt());
}

void ScopeVisitor::visit(AssignStmt* node) {
    resolveVar(node->getName());
    node->getExpression()->accept(this);
}

void ScopeVisitor::visit(PrintStmt* node) {
    node->getExpression()->accept(this);
}

void ScopeVisitor::visit(IfStmt* node) {
    node->getCondition()->accept(this);

    scopeTree_.pushScope(ScopeKind::BLOCK, "if-then");
    for (auto& s : node->getThenBranch()) s->accept(this);
    scopeTree_.popScope();

    if (!node->getElseBranch().empty()) {
        scopeTree_.pushScope(ScopeKind::BLOCK, "if-else");
        for (auto& s : node->getElseBranch()) s->accept(this);
        scopeTree_.popScope();
    }
}

void ScopeVisitor::visit(NumberExpr* /*node*/) {}

void ScopeVisitor::visit(VariableExpr* node) {
    resolveVar(node->getName());
}

void ScopeVisitor::visit(BinaryExpr* node) {
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
}

void ScopeVisitor::visit(EqualExpr* node) {
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
}

void ScopeVisitor::visit(ClassDecl* node) {
    currentClass_ = node->getName();

    Scope* classScope = scopeTree_.pushScope(ScopeKind::CLASS, node->getName());

    const ClassSymbol* cs = symTable_.findClass(node->getName());
    classScope->setClassSymbol(cs);

    if (cs) {
        for (auto& f : cs->fields) {
            VariableSymbol fsym = f;
            classScope->declare(fsym, false);
        }
    }

    for (auto& mth : node->getMethods()) {
        mth->accept(this);
    }

    scopeTree_.popScope();
    currentClass_.clear();
}

void ScopeVisitor::visit(FieldDecl* /*node*/) {}

void ScopeVisitor::visit(MethodDecl* node) {
    currentMethod_ = node->getName();

    Scope* methodScope = scopeTree_.pushScope(
        ScopeKind::METHOD,
        currentClass_ + "::" + node->getName());

    if (!currentClass_.empty()) {
        const ClassSymbol* cs = symTable_.findClass(currentClass_);
        if (cs) {
            const MethodSymbol* ms = cs->findMethod(node->getName());
            methodScope->setMethodSymbol(ms);

            if (ms) {
                for (auto& param : ms->params) {
                    VariableSymbol psym = param;
                    methodScope->declare(psym, false);
                }
            }
        }
    }

    for (auto& stmt : node->getBody()) {
        stmt->accept(this);
    }

    scopeTree_.popScope();
    currentMethod_.clear();
}

void ScopeVisitor::visit(ReturnStmt* node) {
    if (node->getExpression()) {
        node->getExpression()->accept(this);
    }
}

void ScopeVisitor::visit(VarDeclStmt* node) {
    TypeInfo type = resolveType(node->getType());
    declareVar(node->getName(), type);

    if (node->getInitializer()) {
        node->getInitializer()->accept(this);
    }
}

void ScopeVisitor::visit(NewObjectExpr* node) {
    const ClassSymbol* cs = symTable_.findClass(node->getClassName());
    if (!cs) {
        throw std::runtime_error(
            "SemanticError: unknown class '" + node->getClassName() + "'");
    }

    const MethodSymbol* ctor = cs->findMethod(node->getClassName());
    if (!ctor || !ctor->isConstructor) {
        throw std::runtime_error(
            "SemanticError: no constructor for class '" + node->getClassName() + "'");
    }

    if (node->getArgs().size() != ctor->params.size()) {
        throw std::runtime_error(
            "SemanticError: constructor '" + node->getClassName() +
            "' expects " + std::to_string(ctor->params.size()) +
            " argument(s), got " + std::to_string(node->getArgs().size()));
    }

    for (auto& arg : node->getArgs()) {
        arg->accept(this);
    }
}

void ScopeVisitor::visit(NewArrayExpr* node) {
    resolveType(node->getElemType());
    node->getSize()->accept(this);
}

void ScopeVisitor::visit(MethodCallExpr* node) {
    node->getObject()->accept(this);
    for (auto& arg : node->getArgs()) {
        arg->accept(this);
    }
}

void ScopeVisitor::visit(FieldAccessExpr* node) {
    node->getObject()->accept(this);
}

void ScopeVisitor::visit(ThisExpr* /*node*/) {
    if (currentClass_.empty()) {
        throw std::runtime_error(
            "SemanticError: 'this' used outside of a class method");
    }
}

void ScopeVisitor::visit(ArrayAccessExpr* node) {
    node->getArray()->accept(this);
    node->getIndex()->accept(this);
}

void ScopeVisitor::visit(ArrayLengthExpr* node) {
    node->getArray()->accept(this);
}
