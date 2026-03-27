#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "scope_visitor.h"
#include "oop_nodes.h"
#include <stdexcept>
#include <string>

class ScopeVisitorTest : public ::testing::Test {
protected:
    ScopeVisitor analyse(const std::string& src) {
        Lexer lexer(src);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parseProgram();

        ScopeVisitor sv;
        sv.collectClasses(program.get());
        program->accept(&sv);
        return sv;
    }

    const Scope* rootScope(const std::string& src) {
        return analyse(src).getScopeTree().root();
    }
};

TEST_F(ScopeVisitorTest, SymbolTable_EmptyForNoClasses) {
    auto sv = analyse("declare x: int;");
    EXPECT_TRUE(sv.getSymbolTable().allClasses().empty());
}

TEST_F(ScopeVisitorTest, SymbolTable_RegistersOneClass) {
    auto sv = analyse("class Foo { }");
    EXPECT_NE(sv.getSymbolTable().findClass("Foo"), nullptr);
}

TEST_F(ScopeVisitorTest, SymbolTable_RegistersTwoClasses) {
    auto sv = analyse("class A { } class B { }");
    EXPECT_NE(sv.getSymbolTable().findClass("A"), nullptr);
    EXPECT_NE(sv.getSymbolTable().findClass("B"), nullptr);
}

TEST_F(ScopeVisitorTest, SymbolTable_ClassHasCorrectFields) {
    auto sv = analyse("class Point { int x; int y; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Point");
    ASSERT_NE(cs, nullptr);
    EXPECT_EQ(cs->fieldCount(), 2);
    EXPECT_NE(cs->findField("x"), nullptr);
    EXPECT_NE(cs->findField("y"), nullptr);
}

TEST_F(ScopeVisitorTest, SymbolTable_FieldsArePrivate) {
    auto sv = analyse("class Foo { int val; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    ASSERT_NE(cs, nullptr);
    const VariableSymbol* f = cs->findField("val");
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->storage, VariableSymbol::StorageKind::FIELD);
}

TEST_F(ScopeVisitorTest, SymbolTable_FieldIndexIsAssigned) {
    auto sv = analyse("class Foo { int a; int b; int c; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    ASSERT_NE(cs, nullptr);
    EXPECT_EQ(cs->findField("a")->index, 0);
    EXPECT_EQ(cs->findField("b")->index, 1);
    EXPECT_EQ(cs->findField("c")->index, 2);
}

TEST_F(ScopeVisitorTest, SymbolTable_ClassHasMethod) {
    auto sv = analyse("class Foo { int get() { return 1; } }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    ASSERT_NE(cs, nullptr);
    EXPECT_NE(cs->findMethod("get"), nullptr);
}

TEST_F(ScopeVisitorTest, SymbolTable_MethodHasCorrectReturnType) {
    auto sv = analyse("class Foo { int compute() { return 0; } }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    ASSERT_NE(cs, nullptr);
    const MethodSymbol* ms = cs->findMethod("compute");
    ASSERT_NE(ms, nullptr);
    EXPECT_EQ(ms->returnType.kind, VarKind::INT);
}

TEST_F(ScopeVisitorTest, SymbolTable_MethodHasParams) {
    auto sv = analyse("class Calc { int add(int a, int b) { return 0; } }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Calc");
    ASSERT_NE(cs, nullptr);
    const MethodSymbol* ms = cs->findMethod("add");
    ASSERT_NE(ms, nullptr);
    ASSERT_EQ(ms->params.size(), 2);
    EXPECT_EQ(ms->params[0].name, "a");
    EXPECT_EQ(ms->params[1].name, "b");
}

TEST_F(ScopeVisitorTest, SymbolTable_ParamsAreStoredAsParam) {
    auto sv = analyse("class Foo { int f(int p) { return p; } }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    const MethodSymbol* ms = cs->findMethod("f");
    ASSERT_NE(ms, nullptr);
    EXPECT_EQ(ms->params[0].storage, VariableSymbol::StorageKind::PARAM);
}


TEST_F(ScopeVisitorTest, SymbolTable_DefaultConstructorSynthesised) {
    auto sv = analyse("class Foo { int x; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    ASSERT_NE(cs, nullptr);
    const MethodSymbol* ctor = cs->findMethod("Foo");
    ASSERT_NE(ctor, nullptr);
    EXPECT_TRUE(ctor->isConstructor);
}

TEST_F(ScopeVisitorTest, SymbolTable_ConstructorMemoryFootprint) {
    auto sv = analyse("class Vec3 { int x; int y; int z; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Vec3");
    ASSERT_NE(cs, nullptr);
    EXPECT_EQ(cs->fieldCount(), 3);
}

TEST_F(ScopeVisitorTest, SymbolTable_ConstructorOwnerClass) {
    auto sv = analyse("class Bar { }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Bar");
    const MethodSymbol* ctor = cs->findMethod("Bar");
    ASSERT_NE(ctor, nullptr);
    EXPECT_EQ(ctor->ownerClass, "Bar");
}

TEST_F(ScopeVisitorTest, ScopeTree_RootIsGlobal) {
    auto sv = analyse("declare x: int;");
    EXPECT_EQ(sv.getScopeTree().root()->kind(), ScopeKind::GLOBAL);
}

TEST_F(ScopeVisitorTest, ScopeTree_GlobalVarDeclaredInRoot) {
    auto sv = analyse("declare x: int;");
    const Scope* root = sv.getScopeTree().root();
    EXPECT_NE(root->lookupLocal("x"), nullptr);
}

TEST_F(ScopeVisitorTest, ScopeTree_MultipleGlobalVars) {
    auto sv = analyse("declare a: int; declare b: int; declare c: int;");
    const Scope* root = sv.getScopeTree().root();
    EXPECT_NE(root->lookupLocal("a"), nullptr);
    EXPECT_NE(root->lookupLocal("b"), nullptr);
    EXPECT_NE(root->lookupLocal("c"), nullptr);
}

TEST_F(ScopeVisitorTest, ScopeTree_ClassScopeCreated) {
    auto sv = analyse("class Foo { }");
    const Scope* root = sv.getScopeTree().root();
    ASSERT_EQ(root->children().size(), 1);
    EXPECT_EQ(root->children()[0]->kind(), ScopeKind::CLASS);
    EXPECT_EQ(root->children()[0]->label(), "Foo");
}

TEST_F(ScopeVisitorTest, ScopeTree_ClassScopeHasClassSymbol) {
    auto sv = analyse("class Foo { int x; }");
    const Scope* classScope = sv.getScopeTree().root()->children()[0].get();
    EXPECT_NE(classScope->classSymbol(), nullptr);
    EXPECT_EQ(classScope->classSymbol()->name, "Foo");
}

TEST_F(ScopeVisitorTest, ScopeTree_FieldsDeclaredInClassScope) {
    auto sv = analyse("class Point { int x; int y; }");
    const Scope* classScope = sv.getScopeTree().root()->children()[0].get();
    EXPECT_NE(classScope->lookupLocal("x"), nullptr);
    EXPECT_NE(classScope->lookupLocal("y"), nullptr);
}

TEST_F(ScopeVisitorTest, ScopeTree_MethodScopeCreated) {
    auto sv = analyse("class Foo { int get() { return 1; } }");
    const Scope* classScope = sv.getScopeTree().root()->children()[0].get();
    ASSERT_EQ(classScope->children().size(), 1);
    EXPECT_EQ(classScope->children()[0]->kind(), ScopeKind::METHOD);
}

TEST_F(ScopeVisitorTest, ScopeTree_MethodScopeLabel) {
    auto sv = analyse("class Foo { int bar() { return 0; } }");
    const Scope* methodScope =
        sv.getScopeTree().root()->children()[0]->children()[0].get();
    EXPECT_EQ(methodScope->label(), "Foo::bar");
}

TEST_F(ScopeVisitorTest, ScopeTree_MethodScopeHasMethodSymbol) {
    auto sv = analyse("class Foo { int bar() { return 0; } }");
    const Scope* methodScope =
        sv.getScopeTree().root()->children()[0]->children()[0].get();
    EXPECT_NE(methodScope->methodSymbol(), nullptr);
    EXPECT_EQ(methodScope->methodSymbol()->name, "bar");
}

TEST_F(ScopeVisitorTest, ScopeTree_ParamsDeclaredInMethodScope) {
    auto sv = analyse("class Foo { int f(int p) { return p; } }");
    const Scope* methodScope =
        sv.getScopeTree().root()->children()[0]->children()[0].get();
    EXPECT_NE(methodScope->lookupLocal("p"), nullptr);
}

TEST_F(ScopeVisitorTest, ScopeTree_LocalVarDeclaredInMethodScope) {
    auto sv = analyse("class Foo { int f() { int local = 5; return local; } }");
    const Scope* methodScope =
        sv.getScopeTree().root()->children()[0]->children()[0].get();
    EXPECT_NE(methodScope->lookupLocal("local"), nullptr);
}

TEST_F(ScopeVisitorTest, ScopeTree_IfThenCreatesBlockScope) {
    auto sv = analyse("declare x: int; x = 1; if (x == 1) { declare y: int; }");
    const Scope* root = sv.getScopeTree().root();
    bool foundBlock = false;
    for (auto& child : root->children()) {
        if (child->kind() == ScopeKind::BLOCK && child->label() == "if-then") {
            foundBlock = true;
        }
    }
    EXPECT_TRUE(foundBlock);
}

TEST_F(ScopeVisitorTest, ScopeTree_IfElseCreatesTwoBlockScopes) {
    auto sv = analyse(
        "declare x: int; x = 0;"
        "if (x == 1) { declare a: int; } else { declare b: int; }");
    const Scope* root = sv.getScopeTree().root();
    int blockCount = 0;
    for (auto& child : root->children()) {
        if (child->kind() == ScopeKind::BLOCK) blockCount++;
    }
    EXPECT_EQ(blockCount, 2);
}

TEST_F(ScopeVisitorTest, ScopeTree_VarInThenScopeNotVisibleInElse) {
    auto sv = analyse(
        "declare x: int; x = 0;"
        "if (x == 1) { declare a: int; } else { declare b: int; }");
    const Scope* root = sv.getScopeTree().root();
    const Scope* thenScope = nullptr;
    const Scope* elseScope = nullptr;
    for (auto& child : root->children()) {
        if (child->kind() == ScopeKind::BLOCK && child->label() == "if-then")
            thenScope = child.get();
        if (child->kind() == ScopeKind::BLOCK && child->label() == "if-else")
            elseScope = child.get();
    }
    ASSERT_NE(thenScope, nullptr);
    ASSERT_NE(elseScope, nullptr);
    EXPECT_NE(thenScope->lookupLocal("a"), nullptr);
    EXPECT_EQ(thenScope->lookupLocal("b"), nullptr);
    EXPECT_NE(elseScope->lookupLocal("b"), nullptr);
    EXPECT_EQ(elseScope->lookupLocal("a"), nullptr);
}

TEST_F(ScopeVisitorTest, Lookup_GlobalVarResolvedInAssignment) {
    EXPECT_NO_THROW(analyse("declare x: int; x = 5;"));
}

TEST_F(ScopeVisitorTest, Lookup_GlobalVarResolvedInExpression) {
    EXPECT_NO_THROW(analyse("declare x: int; declare y: int; x = 1; y = x + 2;"));
}

TEST_F(ScopeVisitorTest, Lookup_VarInIfCondition) {
    EXPECT_NO_THROW(analyse(
        "declare x: int; x = 3;"
        "if (x == 3) { print(1); }"));
}

TEST_F(ScopeVisitorTest, Lookup_VarDeclaredInOuterScopeVisibleInBlock) {
    EXPECT_NO_THROW(analyse(
        "declare x: int; x = 10;"
        "if (x == 10) { print(x); }"));
}

TEST_F(ScopeVisitorTest, Lookup_FieldVisibleInMethodBody) {
    EXPECT_NO_THROW(analyse(R"(
        class Counter {
            int count;
            int increment() {
                int newVal = count + 1;
                return newVal;
            }
        }
    )"));
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableInAssignment) {
    EXPECT_THROW(analyse("z = 5;"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableInExpression) {
    EXPECT_THROW(analyse("declare x: int; x = y + 1;"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableInPrint) {
    EXPECT_THROW(analyse("print(undeclared);"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableInCondition) {
    EXPECT_THROW(analyse("if (ghost == 1) { print(0); }"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableInIfBody) {
    EXPECT_THROW(analyse(
        "declare x: int; x = 1;"
        "if (x == 1) { print(missing); }"),
        std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_UndeclaredVariableErrorMessage) {
    try {
        analyse("declare a: int; a = b;");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("'b'"), std::string::npos);
        EXPECT_NE(msg.find("SemanticError"), std::string::npos);
    }
}

TEST_F(ScopeVisitorTest, Error_DoubleDeclareGlobalScope) {
    EXPECT_THROW(analyse("declare x: int; declare x: int;"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_DoubleDeclareTypedVarGlobalScope) {
    EXPECT_THROW(analyse("int x; int x;"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_DoubleDeclareInBlock) {
    EXPECT_THROW(analyse(
        "declare cond: int; cond = 1;"
        "if (cond == 1) { declare y: int; declare y: int; }"),
        std::runtime_error);
}

TEST_F(ScopeVisitorTest, Error_DoubleDeclareErrorMessage) {
    try {
        analyse("declare x: int; declare x: int;");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("'x'"), std::string::npos);
        EXPECT_NE(msg.find("already declared"), std::string::npos);
    }
}

TEST_F(ScopeVisitorTest, Error_DoubleDeclareInMethodBody) {
    EXPECT_THROW(analyse(R"(
        class Foo {
            int f() {
                int local;
                int local;
                return local;
            }
        }
    )"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, Shadowing_AllowedInNestedBlock) {
    EXPECT_NO_THROW(analyse(
        "declare x: int; x = 5;"
        "if (x == 5) { declare x: int; x = 99; }"));
}

TEST_F(ScopeVisitorTest, Shadowing_OuterVarStillAccessibleAfterBlock) {
    auto sv = analyse(
        "declare x: int; x = 5;"
        "if (x == 5) { declare x: int; x = 99; }");
    const Scope* root = sv.getScopeTree().root();
    EXPECT_NE(root->lookupLocal("x"), nullptr);
    const Scope* thenScope = nullptr;
    for (auto& child : root->children()) {
        if (child->kind() == ScopeKind::BLOCK) { thenScope = child.get(); break; }
    }
    ASSERT_NE(thenScope, nullptr);
    EXPECT_NE(thenScope->lookupLocal("x"), nullptr);
}

TEST_F(ScopeVisitorTest, Shadowing_DifferentVarsNoWarning) {
    EXPECT_NO_THROW(analyse(
        "declare a: int; a = 1;"
        "if (a == 1) { declare b: int; b = 2; }"));
}

TEST_F(ScopeVisitorTest, TypeInfo_IntField) {
    auto sv = analyse("class Foo { int x; }");
    const ClassSymbol* cs = sv.getSymbolTable().findClass("Foo");
    EXPECT_EQ(cs->findField("x")->type.kind, VarKind::INT);
}

TEST_F(ScopeVisitorTest, TypeInfo_IntToString) {
    TypeInfo t = TypeInfo::makeInt();
    EXPECT_EQ(t.toString(), "int");
}

TEST_F(ScopeVisitorTest, TypeInfo_ClassToString) {
    TypeInfo t = TypeInfo::makeClass("Point");
    EXPECT_EQ(t.toString(), "Point");
}

TEST_F(ScopeVisitorTest, TypeInfo_ArrayIntToString) {
    TypeInfo t = TypeInfo::makeArrayInt();
    EXPECT_EQ(t.toString(), "int[]");
}

TEST_F(ScopeVisitorTest, TypeInfo_ArrayClassToString) {
    TypeInfo t = TypeInfo::makeArrayClass("Node");
    EXPECT_EQ(t.toString(), "Node[]");
}

TEST_F(ScopeVisitorTest, ScopeTree_LookupWalksParentChain) {
    ScopeTree tree;
    VariableSymbol outer("x", TypeInfo::makeInt());
    tree.declare(outer);

    tree.pushScope(ScopeKind::BLOCK, "inner");
    const VariableSymbol* found = tree.current()->lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "x");
    tree.popScope();
}

TEST_F(ScopeVisitorTest, ScopeTree_LookupThrowsForMissing) {
    ScopeTree tree;
    EXPECT_THROW(tree.lookup("nonexistent"), std::runtime_error);
}

TEST_F(ScopeVisitorTest, ScopeTree_PopScopeRestoresCurrent) {
    ScopeTree tree;
    Scope* root = tree.current();
    tree.pushScope(ScopeKind::BLOCK, "b");
    tree.popScope();
    EXPECT_EQ(tree.current(), root);
}

TEST_F(ScopeVisitorTest, ScopeTree_CannotPopRoot) {
    ScopeTree tree;
    EXPECT_THROW(tree.popScope(), std::logic_error);
}

TEST_F(ScopeVisitorTest, ScopeTree_DoubleDeclareThrows) {
    ScopeTree tree;
    VariableSymbol sym("x", TypeInfo::makeInt());
    tree.declare(sym, false);
    EXPECT_THROW(tree.declare(sym, false), std::runtime_error);
}
