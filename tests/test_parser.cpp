#include <gtest/gtest.h>
#include "parser.h"
#include "lexer.h"
#include "context.h"
#include "oop_nodes.h"
#include <sstream>
#include <iostream>

class ParserTest : public ::testing::Test {
protected:
    Parser createParser(const std::string& input) {
        Lexer lexer(input);
        std::vector<Token> tokens = lexer.tokenize();
        return Parser(tokens);
    }
    
    void captureOutput(std::function<void()> func, std::string& output) {
        std::stringstream buffer;
        std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
        
        func();
        
        std::cout.rdbuf(old);
        output = buffer.str();
    }
};

TEST_F(ParserTest, ParseSimpleDeclaration) {
    Parser parser = createParser("declare x: int;");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, ParseMultipleDeclarations) {
    Parser parser = createParser("declare x: int; declare y: int;");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 2);
}

TEST_F(ParserTest, ParseAssignment) {
    Parser parser = createParser("x = 5;");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, ParseSimpleNumber) {
    Parser parser = createParser("x = 42;");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, ExecuteDeclaration) {
    Parser parser = createParser("declare x: int;");
    auto program = parser.parseProgram();
    Context ctx;
    
    program->execute(ctx);

    EXPECT_EQ(ctx.get("x"), 0);
}

TEST_F(ParserTest, ExecuteAssignment) {
    Parser parser = createParser("declare x: int; x = 42;");
    auto program = parser.parseProgram();
    Context ctx;
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 42);
}

TEST_F(ParserTest, ExecutePrintStatement) {
    Parser parser = createParser("print(123);");
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "123\n");
}

TEST_F(ParserTest, ExecutePrintVariable) {
    Parser parser = createParser("declare x: int; x = 99; print(x);");
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "99\n");
}

TEST_F(ParserTest, ParseEqualityExpression) {
    Parser parser = createParser("x = 5 == 5;");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, EvaluateEqualityTrue) {
    Parser parser = createParser("x = 5 == 5;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 1);
}

TEST_F(ParserTest, EvaluateEqualityFalse) {
    Parser parser = createParser("x = 5 == 3;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 0);
}

TEST_F(ParserTest, ComplexProgram) {
    std::string code = R"(
        declare x: int;
        declare y: int;
        x = 10;
        y = 20;
        print(x);
        print(y);
    )";
    
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "10\n20\n");
}

TEST_F(ParserTest, AssignmentWithExpression) {
    Parser parser = createParser("declare x: int; x = 5 == 5; print(x);");
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "1\n");
}

TEST_F(ParserTest, InvalidDeclarationMissingColon) {
    Parser parser = createParser("declare x int;");
    EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST_F(ParserTest, InvalidStatementEmptyCode) {
    Parser parser = createParser("");
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 0);
}

TEST_F(ParserTest, ParseIfStatement) {
    std::string code = "if (1 == 1) { print(10); }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, ParseIfElseStatement) {
    std::string code = "if (1 == 1) { print(10); } else { print(20); }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 1);
}

TEST_F(ParserTest, ExecuteIfTrueBranch) {
    std::string code = "if (1 == 1) { print(99); }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "99\n");
}

TEST_F(ParserTest, ExecuteIfFalseBranch) {
    std::string code = "if (0 == 1) { print(99); } else { print(55); }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "55\n");
}

TEST_F(ParserTest, ExecuteIfWithVariables) {
    std::string code = R"(
        declare x: int;
        x = 42;
        if (x == 42) {
            print(100);
        } else {
            print(200);
        }
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "100\n");
}

TEST_F(ParserTest, ExecuteIfNoElse) {
    std::string code = R"(
        declare x: int;
        x = 5;
        if (x == 10) {
            print(50);
        }
        print(75);
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "75\n");
}

TEST_F(ParserTest, NestedIfStatements) {
    std::string code = R"(
        declare x: int;
        x = 10;
        if (x == 10) {
            print(1);
            if (x == 10) {
                print(2);
            }
        }
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "1\n2\n");
}

TEST_F(ParserTest, ParseAddition) {
    Parser parser = createParser("x = 5 + 3;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 8);
}

TEST_F(ParserTest, ParseSubtraction) {
    Parser parser = createParser("x = 10 - 3;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 7);
}

TEST_F(ParserTest, ParseMultiplication) {
    Parser parser = createParser("x = 4 * 5;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 20);
}

TEST_F(ParserTest, ParseDivision) {
    Parser parser = createParser("x = 20 / 4;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 5);
}

TEST_F(ParserTest, OperatorPrecedence) {
    Parser parser = createParser("x = 2 + 3 * 4;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 14);  // 2 + (3 * 4) = 14
}

TEST_F(ParserTest, ParenthesesOverridePrecedence) {
    Parser parser = createParser("x = (2 + 3) * 4;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 20);  // (2 + 3) * 4 = 20
}

TEST_F(ParserTest, ComplexExpression) {
    Parser parser = createParser("x = 100 - 20 / 4 + 3 * 2;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 101);  // 100 - 5 + 6 = 101
}

TEST_F(ParserTest, DivisionByZero) {
    Parser parser = createParser("x = 10 / 0;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    EXPECT_THROW(program->execute(ctx), std::runtime_error);
}

TEST_F(ParserTest, ArithmeticInCondition) {
    std::string code = R"(
        if (10 + 5 == 15) {
            print(1);
        } else {
            print(0);
        }
    )";
    
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "1\n");
}

TEST_F(ParserTest, VariablesInArithmetic) {
    std::string code = R"(
        declare a: int;
        declare b: int;
        declare c: int;
        a = 10;
        b = 5;
        c = a + b * 2;
        print(c);
    )";
    
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    Context ctx;
    
    std::string output;
    captureOutput([&]() { program->execute(ctx); }, output);
    
    EXPECT_EQ(output, "20\n");  // 10 + (5 * 2) = 20
}

TEST_F(ParserTest, ParseEmptyClass) {
    std::string code = "class Foo { }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    ASSERT_EQ(program->statements.size(), 1);
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->getName(), "Foo");
    EXPECT_EQ(cls->getFields().size(), 0);
    EXPECT_EQ(cls->getMethods().size(), 0);
}

TEST_F(ParserTest, ParseClassWithOneField) {
    std::string code = "class Point { int x; }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(cls->getFields().size(), 1);
    EXPECT_EQ(cls->getFields()[0]->getName(), "x");
    EXPECT_EQ(cls->getFields()[0]->getType().typeName, "int");
}

TEST_F(ParserTest, ParseClassWithMultipleFields) {
    std::string code = "class Point { int x; int y; }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->getFields().size(), 2);
    EXPECT_EQ(cls->getFields()[0]->getName(), "x");
    EXPECT_EQ(cls->getFields()[1]->getName(), "y");
}

TEST_F(ParserTest, ParseClassWithMethod) {
    std::string code = "class Foo { int getValue() { return 42; } }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(cls->getMethods().size(), 1);
    EXPECT_EQ(cls->getMethods()[0]->getName(), "getValue");
    EXPECT_EQ(cls->getMethods()[0]->getReturnType().typeName, "int");
    EXPECT_EQ(cls->getMethods()[0]->getParams().size(), 0);
}

TEST_F(ParserTest, ParseMethodWithParams) {
    std::string code = "class Calc { int add(int a, int b) { return a + b; } }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(cls->getMethods().size(), 1);
    auto& method = cls->getMethods()[0];
    EXPECT_EQ(method->getName(), "add");
    ASSERT_EQ(method->getParams().size(), 2);
    EXPECT_EQ(method->getParams()[0].name, "a");
    EXPECT_EQ(method->getParams()[1].name, "b");
}

TEST_F(ParserTest, ParseClassWithFieldsAndMethods) {
    std::string code = R"(
        class Counter {
            int count;
            int increment(int delta) {
                return count + delta;
            }
        }
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->getFields().size(), 1);
    EXPECT_EQ(cls->getMethods().size(), 1);
}

TEST_F(ParserTest, ParseTypedVarDecl) {
    std::string code = "int x;";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    ASSERT_EQ(program->statements.size(), 1);
    auto* decl = dynamic_cast<VarDeclStmt*>(program->statements[0].get());
    ASSERT_NE(decl, nullptr);
    EXPECT_EQ(decl->getName(), "x");
    EXPECT_EQ(decl->getType().typeName, "int");
    EXPECT_EQ(decl->getInitializer(), nullptr);
}

TEST_F(ParserTest, ParseTypedVarDeclWithInit) {
    std::string code = "int x = 5;";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* decl = dynamic_cast<VarDeclStmt*>(program->statements[0].get());
    ASSERT_NE(decl, nullptr);
    EXPECT_EQ(decl->getName(), "x");
    EXPECT_NE(decl->getInitializer(), nullptr);
}

TEST_F(ParserTest, ParseReturnStatement) {
    std::string code = "class Foo { int get() { return 1; } }";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    auto* cls = dynamic_cast<ClassDecl*>(program->statements[0].get());
    ASSERT_NE(cls, nullptr);
    auto& body = cls->getMethods()[0]->getBody();
    ASSERT_EQ(body.size(), 1);
    auto* ret = dynamic_cast<ReturnStmt*>(body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_NE(ret->getExpression(), nullptr);
}

TEST_F(ParserTest, ParseNewObjectExpression) {
    std::string code = "class Foo { } int x = 0; x = 0;";
    std::string newCode = "class Point { int x; } int y = 0;";
    Parser parser = createParser(newCode);
    EXPECT_NO_THROW(parser.parseProgram());
}

TEST_F(ParserTest, ParseMultipleClasses) {
    std::string code = R"(
        class A { int x; }
        class B { int y; }
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 2);
    auto* a = dynamic_cast<ClassDecl*>(program->statements[0].get());
    auto* b = dynamic_cast<ClassDecl*>(program->statements[1].get());
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(a->getName(), "A");
    EXPECT_EQ(b->getName(), "B");
}

TEST_F(ParserTest, ParseClassFollowedByStatements) {
    std::string code = R"(
        class Foo { int val; }
        declare x: int;
        x = 10;
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 3);
}

TEST_F(ParserTest, ParseCommentedCode) {
    std::string code = R"(
        // this is a comment
        declare x: int;
        // another comment
        x = 5;
    )";
    Parser parser = createParser(code);
    auto program = parser.parseProgram();
    EXPECT_EQ(program->statements.size(), 2);
}
