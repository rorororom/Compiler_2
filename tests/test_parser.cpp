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
