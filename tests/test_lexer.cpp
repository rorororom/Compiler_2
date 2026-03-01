#include <gtest/gtest.h>
#include "lexer.h"

class LexerTest : public ::testing::Test {
protected:
    Lexer createLexer(const std::string& input) {
        return Lexer(input);
    }
};

TEST_F(LexerTest, TokenizeNumber) {
    Lexer lexer = createLexer("42");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::NUMBER);
    EXPECT_EQ(token.lexeme, "42");
}

TEST_F(LexerTest, TokenizeIdentifier) {
    Lexer lexer = createLexer("myVar");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::ID);
    EXPECT_EQ(token.lexeme, "myVar");
}

TEST_F(LexerTest, TokenizeKeywordDeclare) {
    Lexer lexer = createLexer("declare");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::DECLARE);
}

TEST_F(LexerTest, TokenizeKeywordInt) {
    Lexer lexer = createLexer("int");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::INT);
}

TEST_F(LexerTest, TokenizeKeywordPrint) {
    Lexer lexer = createLexer("print");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::PRINT);
}

TEST_F(LexerTest, TokenizeKeywordIf) {
    Lexer lexer = createLexer("if");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::IF);
}

TEST_F(LexerTest, TokenizeAssignOperator) {
    Lexer lexer = createLexer("=");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::ASSIGN);
}

TEST_F(LexerTest, TokenizeEqualOperator) {
    Lexer lexer = createLexer("==");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::EQUAL);
}

TEST_F(LexerTest, TokenizeColon) {
    Lexer lexer = createLexer(":");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::COLON);
}

TEST_F(LexerTest, TokenizeSemicolon) {
    Lexer lexer = createLexer(";");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::SEMICOLON);
}

TEST_F(LexerTest, TokenizeLeftParen) {
    Lexer lexer = createLexer("(");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LPAREN);
}

TEST_F(LexerTest, TokenizeRightParen) {
    Lexer lexer = createLexer(")");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::RPAREN);
}

TEST_F(LexerTest, TokenizeLeftBrace) {
    Lexer lexer = createLexer("{");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LBRACE);
}

TEST_F(LexerTest, TokenizeRightBrace) {
    Lexer lexer = createLexer("}");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::RBRACE);
}

TEST_F(LexerTest, TokenizeDeclarationStatement) {
    Lexer lexer = createLexer("declare x: int;");
    
    Token token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::DECLARE);
    
    Token token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::ID);
    EXPECT_EQ(token2.lexeme, "x");
    
    Token token3 = lexer.nextToken();
    EXPECT_EQ(token3.type, TokenType::COLON);
    
    Token token4 = lexer.nextToken();
    EXPECT_EQ(token4.type, TokenType::INT);
    
    Token token5 = lexer.nextToken();
    EXPECT_EQ(token5.type, TokenType::SEMICOLON);
    
    Token eof = lexer.nextToken();
    EXPECT_EQ(eof.type, TokenType::END);
}

TEST_F(LexerTest, TokenizeAssignmentStatement) {
    Lexer lexer = createLexer("x = 5;");
    
    Token token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::ID);
    EXPECT_EQ(token1.lexeme, "x");
    
    Token token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::ASSIGN);
    
    Token token3 = lexer.nextToken();
    EXPECT_EQ(token3.type, TokenType::NUMBER);
    EXPECT_EQ(token3.lexeme, "5");
    
    Token token4 = lexer.nextToken();
    EXPECT_EQ(token4.type, TokenType::SEMICOLON);
}

TEST_F(LexerTest, SkipsWhitespace) {
    Lexer lexer = createLexer("  x  =  5  ;  ");
    
    Token token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::ID);
    EXPECT_EQ(token1.lexeme, "x");
    
    Token token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::ASSIGN);
    
    Token token3 = lexer.nextToken();
    EXPECT_EQ(token3.type, TokenType::NUMBER);
    EXPECT_EQ(token3.lexeme, "5");
}

TEST_F(LexerTest, TokenizeMultiDigitNumber) {
    Lexer lexer = createLexer("12345");
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::NUMBER);
    EXPECT_EQ(token.lexeme, "12345");
}
