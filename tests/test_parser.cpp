#include <gtest/gtest.h>
#include "parser.h"
#include "lexer.h"
#include "context.h"
#include <sstream>
#include <iostream>

class ParserTest : public ::testing::Test {
protected:
    Parser createParser(const std::string& input) {
        Lexer lexer(input);
        return Parser(lexer);
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
    
    // After declaration, variable should exist with value 0
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
    
    EXPECT_EQ(ctx.get("x"), 1);  // True is 1
}

TEST_F(ParserTest, EvaluateEqualityFalse) {
    Parser parser = createParser("x = 5 == 3;");
    auto program = parser.parseProgram();
    Context ctx;
    ctx.declare("x");
    
    program->execute(ctx);
    
    EXPECT_EQ(ctx.get("x"), 0);  // False is 0
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
