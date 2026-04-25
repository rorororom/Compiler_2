#include "scope_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
#include "program.h"
#include <iostream>
#include <stdexcept>

ScopeVisitor::ScopeVisitor() {
    registerHandler<Program>([this](Program* node) {
        for (auto& stmt : node->getStatements())
            stmt->accept(this);
    });

    registerHandler<DeclareStmt>([this](DeclareStmt* node) {
        declareVar(node->getName(), TypeInfo::makeInt());
    });

    registerHandler<AssignStmt>([this](AssignStmt* node) {
        resolveVar(node->getName());
        node->getExpression()->accept(this);
    });

    registerHandler<PrintStmt>([this](PrintStmt* node) {
        node->getExpression()->accept(this);
    });

    registerHandler<IfStmt>([this](IfStmt* node) {
        node->getCondition()->accept(this);

        scopeTree_.pushScope(ScopeKind::BLOCK, "if-then");
        for (auto& s : node->getThenBranch()) s->accept(this);
        scopeTree_.popScope();

        if (!node->getElseBranch().empty()) {
            scopeTree_.pushScope(ScopeKind::BLOCK, "if-else");
            for (auto& s : node->getElseBranch()) s->accept(this);
            scopeTree_.popScope();
        }
    });

    registerHandler<NumberExpr>([](NumberExpr*) {});

    registerHandler<VariableExpr>([this](VariableExpr* node) {
        resolveVar(node->getName());
    });

    registerHandler<BinaryExpr>([this](BinaryExpr* node) {
        node->getLeft()->accept(this);
        node->getRight()->accept(this);
    });

    registerHandler<EqualExpr>([this](EqualExpr* node) {
        node->getLeft()->accept(this);
        node->getRight()->accept(this);
    });

    registerHandler<ClassDecl>([this](ClassDecl* node) {
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

        for (auto& mth : node->getMethods())
            mth->accept(this);

        scopeTree_.popScope();
        currentClass_.clear();
    });

    registerHandler<FieldDecl>([](FieldDecl*) {});

    registerHandler<MethodDecl>([this](MethodDecl* node) {
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
                    for (const auto& param : ms->params)
                        methodScope->declare(param, false);
                }
            }
        }

        for (auto& stmt : node->getBody())
            stmt->accept(this);

        scopeTree_.popScope();
        currentMethod_.clear();
    });

    registerHandler<ReturnStmt>([this](ReturnStmt* node) {
        if (node->getExpression())
            node->getExpression()->accept(this);
    });

    registerHandler<VarDeclStmt>([this](VarDeclStmt* node) {
        TypeInfo type = resolveType(node->getType());
        declareVar(node->getName(), type);
        if (node->getInitializer())
            node->getInitializer()->accept(this);
    });

    registerHandler<NewObjectExpr>([this](NewObjectExpr* node) {
        const ClassSymbol* cs = symTable_.findClass(node->getClassName());
        if (!cs)
            throw std::runtime_error(
                "SemanticError: unknown class '" + node->getClassName() + "'");

        const MethodSymbol* ctor = cs->findMethod(node->getClassName());
        if (!ctor || !ctor->isConstructor)
            throw std::runtime_error(
                "SemanticError: no constructor for class '" + node->getClassName() + "'");

        if (node->getArgs().size() != ctor->params.size())
            throw std::runtime_error(
                "SemanticError: constructor '" + node->getClassName() +
                "' expects " + std::to_string(ctor->params.size()) +
                " argument(s), got " + std::to_string(node->getArgs().size()));

        for (auto& arg : node->getArgs())
            arg->accept(this);
    });

    registerHandler<NewArrayExpr>([this](NewArrayExpr* node) {
        resolveType(node->getElemType());
        node->getSize()->accept(this);
    });

    registerHandler<MethodCallExpr>([this](MethodCallExpr* node) {
        node->getObject()->accept(this);

        if (auto* varExpr = dynamic_cast<VariableExpr*>(node->getObject())) {
            const VariableSymbol* sym = scopeTree_.current()->lookup(varExpr->getName());
            if (sym && (sym->type.kind == VarKind::CLASS_INST ||
                        sym->type.kind == VarKind::ARRAY_CLASS)) {
                const ClassSymbol* cs = symTable_.findClass(sym->type.className);
                if (cs) {
                    const MethodSymbol* ms = cs->findMethod(node->getMethodName());
                    if (!ms)
                        throw std::runtime_error(
                            "SemanticError: class '" + sym->type.className +
                            "' has no method '" + node->getMethodName() + "'");
                    if (node->getArgs().size() != ms->params.size())
                        throw std::runtime_error(
                            "SemanticError: method '" + sym->type.className +
                            "::" + node->getMethodName() + "' expects " +
                            std::to_string(ms->params.size()) + " argument(s), got " +
                            std::to_string(node->getArgs().size()));
                }
            }
        }

        for (auto& arg : node->getArgs())
            arg->accept(this);
    });

    registerHandler<FieldAccessExpr>([this](FieldAccessExpr* node) {
        node->getObject()->accept(this);

        if (auto* varExpr = dynamic_cast<VariableExpr*>(node->getObject())) {
            const VariableSymbol* sym = scopeTree_.current()->lookup(varExpr->getName());
            if (sym && sym->type.kind == VarKind::CLASS_INST) {
                const ClassSymbol* cs = symTable_.findClass(sym->type.className);
                if (cs && !cs->findField(node->getFieldName()))
                    throw std::runtime_error(
                        "SemanticError: class '" + sym->type.className +
                        "' has no field '" + node->getFieldName() + "'");
            }
        }
    });

    registerHandler<ThisExpr>([this](ThisExpr*) {
        if (currentClass_.empty())
            throw std::runtime_error(
                "SemanticError: 'this' used outside of a class method");
    });

    registerHandler<ArrayAccessExpr>([this](ArrayAccessExpr* node) {
        node->getArray()->accept(this);
        node->getIndex()->accept(this);
    });

    registerHandler<ArrayLengthExpr>([this](ArrayLengthExpr* node) {
        node->getArray()->accept(this);
    });
}

TypeInfo ScopeVisitor::resolveType(const TypeInfo& ann) const {
    if (ann.kind == VarKind::CLASS_INST || ann.kind == VarKind::ARRAY_CLASS) {
        if (!symTable_.findClass(ann.className))
            throw std::runtime_error(
                "SemanticError: unknown type '" + ann.className + "'");
    }
    return ann;
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
                classSym.addField(VariableSymbol(fld->getName(), ftype,
                                                 VariableSymbol::StorageKind::FIELD));
            }

            for (auto& mth : cls->getMethods()) {
                MethodSymbol msym = mth->getSymbol();
                msym.ownerClass = cls->getName();
                for (auto& p : msym.params)
                    resolveType(p.type);
                resolveType(msym.returnType);
                classSym.addMethod(std::move(msym));
            }

            classSym.ensureConstructor();
            symTable_.addClass(std::move(classSym));
        }
    }
}
