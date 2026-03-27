#include <gtest/gtest.h>
#include "lexer.h"

class LexerTest : public ::testing::Test {
protected:
    Lexer createLexer(const std::string& input) {
        return Lexer(input);
    }

    Token getTokenAt(const std::string& input, size_t index) {
        Lexer lexer(input);
        std::vector<Token> tokens = lexer.tokenize();
        if (index >= tokens.size()) {
            throw std::runtime_error("Token index out of range");
        }
        return tokens[index];
    }

    std::vector<Token> getAllTokens(const std::string& input) {
        Lexer lexer(input);
        return lexer.tokenize();
    }
};

TEST_F(LexerTest, TokenizeNumber) {
    Token token = getTokenAt("42", 0);
    EXPECT_EQ(token.type, TokenType::NUMBER);
    EXPECT_EQ(token.lexeme, "42");
}

TEST_F(LexerTest, TokenizeIdentifier) {
    Token token = getTokenAt("myVar", 0);
    EXPECT_EQ(token.type, TokenType::ID);
    EXPECT_EQ(token.lexeme, "myVar");
}

TEST_F(LexerTest, TokenizeKeywordDeclare) {
    Token token = getTokenAt("declare", 0);
    EXPECT_EQ(token.type, TokenType::DECLARE);
}

TEST_F(LexerTest, TokenizeKeywordInt) {
    Token token = getTokenAt("int", 0);
    EXPECT_EQ(token.type, TokenType::INT);
}

TEST_F(LexerTest, TokenizeKeywordPrint) {
    Token token = getTokenAt("print", 0);
    EXPECT_EQ(token.type, TokenType::PRINT);
}

TEST_F(LexerTest, TokenizeKeywordIf) {
    Token token = getTokenAt("if", 0);
    EXPECT_EQ(token.type, TokenType::IF);
}

TEST_F(LexerTest, TokenizeAssignOperator) {
    Token token = getTokenAt("=", 0);
    EXPECT_EQ(token.type, TokenType::ASSIGN);
}

TEST_F(LexerTest, TokenizeEqualOperator) {
    Token token = getTokenAt("==", 0);
    EXPECT_EQ(token.type, TokenType::EQUAL);
}

TEST_F(LexerTest, TokenizeColon) {
    Token token = getTokenAt(":", 0);
    EXPECT_EQ(token.type, TokenType::COLON);
}

TEST_F(LexerTest, TokenizeSemicolon) {
    Token token = getTokenAt(";", 0);
    EXPECT_EQ(token.type, TokenType::SEMICOLON);
}

TEST_F(LexerTest, TokenizeLeftParen) {
    Token token = getTokenAt("(", 0);
    EXPECT_EQ(token.type, TokenType::LPAREN);
}

TEST_F(LexerTest, TokenizeRightParen) {
    Token token = getTokenAt(")", 0);
    EXPECT_EQ(token.type, TokenType::RPAREN);
}

TEST_F(LexerTest, TokenizeLeftBrace) {
    Token token = getTokenAt("{", 0);
    EXPECT_EQ(token.type, TokenType::LBRACE);
}

TEST_F(LexerTest, TokenizeRightBrace) {
    Token token = getTokenAt("}", 0);
    EXPECT_EQ(token.type, TokenType::RBRACE);
}

TEST_F(LexerTest, TokenizeDeclarationStatement) {
    std::vector<Token> tokens = getAllTokens("declare x: int;");
    
    ASSERT_GE(tokens.size(), 6);
    
    EXPECT_EQ(tokens[0].type, TokenType::DECLARE);
    
    EXPECT_EQ(tokens[1].type, TokenType::ID);
    EXPECT_EQ(tokens[1].lexeme, "x");
    
    EXPECT_EQ(tokens[2].type, TokenType::COLON);
    
    EXPECT_EQ(tokens[3].type, TokenType::INT);
    
    EXPECT_EQ(tokens[4].type, TokenType::SEMICOLON);
    
    EXPECT_EQ(tokens[5].type, TokenType::END);
}

TEST_F(LexerTest, TokenizeAssignmentStatement) {
    std::vector<Token> tokens = getAllTokens("x = 5;");
    
    ASSERT_GE(tokens.size(), 4);
    
    EXPECT_EQ(tokens[0].type, TokenType::ID);
    EXPECT_EQ(tokens[0].lexeme, "x");
    
    EXPECT_EQ(tokens[1].type, TokenType::ASSIGN);
    
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[2].lexeme, "5");
    
    EXPECT_EQ(tokens[3].type, TokenType::SEMICOLON);
}

TEST_F(LexerTest, SkipsWhitespace) {
    std::vector<Token> tokens = getAllTokens("  x  =  5  ;  ");
    
    ASSERT_GE(tokens.size(), 4);
    
    EXPECT_EQ(tokens[0].type, TokenType::ID);
    EXPECT_EQ(tokens[0].lexeme, "x");
    
    EXPECT_EQ(tokens[1].type, TokenType::ASSIGN);
    
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[2].lexeme, "5");
}

TEST_F(LexerTest, TokenizeMultiDigitNumber) {
    Token token = getTokenAt("12345", 0);
    EXPECT_EQ(token.type, TokenType::NUMBER);
    EXPECT_EQ(token.lexeme, "12345");
}

TEST_F(LexerTest, TokenizeKeywordClass) {
    Token token = getTokenAt("class", 0);
    EXPECT_EQ(token.type, TokenType::CLASS);
    EXPECT_EQ(token.lexeme, "class");
}

TEST_F(LexerTest, TokenizeKeywordNew) {
    Token token = getTokenAt("new", 0);
    EXPECT_EQ(token.type, TokenType::NEW);
    EXPECT_EQ(token.lexeme, "new");
}

TEST_F(LexerTest, TokenizeKeywordThis) {
    Token token = getTokenAt("this", 0);
    EXPECT_EQ(token.type, TokenType::THIS);
    EXPECT_EQ(token.lexeme, "this");
}

TEST_F(LexerTest, TokenizeKeywordReturn) {
    Token token = getTokenAt("return", 0);
    EXPECT_EQ(token.type, TokenType::RETURN);
    EXPECT_EQ(token.lexeme, "return");
}

TEST_F(LexerTest, TokenizeKeywordVoid) {
    Token token = getTokenAt("void", 0);
    EXPECT_EQ(token.type, TokenType::VOID);
    EXPECT_EQ(token.lexeme, "void");
}

TEST_F(LexerTest, TokenizeComma) {
    Token token = getTokenAt(",", 0);
    EXPECT_EQ(token.type, TokenType::COMMA);
    EXPECT_EQ(token.lexeme, ",");
}

TEST_F(LexerTest, TokenizeDot) {
    Token token = getTokenAt(".", 0);
    EXPECT_EQ(token.type, TokenType::DOT);
    EXPECT_EQ(token.lexeme, ".");
}

TEST_F(LexerTest, TokenizeLeftBracket) {
    Token token = getTokenAt("[", 0);
    EXPECT_EQ(token.type, TokenType::LBRACKET);
    EXPECT_EQ(token.lexeme, "[");
}

TEST_F(LexerTest, TokenizeRightBracket) {
    Token token = getTokenAt("]", 0);
    EXPECT_EQ(token.type, TokenType::RBRACKET);
    EXPECT_EQ(token.lexeme, "]");
}

TEST_F(LexerTest, SkipsLineComment) {
    std::vector<Token> tokens = getAllTokens("// this is a comment\n42");
    ASSERT_GE(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].lexeme, "42");
    EXPECT_EQ(tokens[1].type, TokenType::END);
}

TEST_F(LexerTest, SkipsInlineComment) {
    std::vector<Token> tokens = getAllTokens("declare x: int; // declare y: int;");
    ASSERT_GE(tokens.size(), 6);
    EXPECT_EQ(tokens[0].type, TokenType::DECLARE);
    EXPECT_EQ(tokens[5].type, TokenType::END);
}

TEST_F(LexerTest, SkipsMultipleCommentLines) {
    std::string code = "// line 1\n// line 2\nint";
    std::vector<Token> tokens = getAllTokens(code);
    ASSERT_GE(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::INT);
}


TEST_F(LexerTest, TokenizeClassDeclaration) {
    std::vector<Token> tokens = getAllTokens("class Point { int x; }");
    ASSERT_GE(tokens.size(), 8);
    EXPECT_EQ(tokens[0].type, TokenType::CLASS);
    EXPECT_EQ(tokens[1].type, TokenType::ID);
    EXPECT_EQ(tokens[1].lexeme, "Point");
    EXPECT_EQ(tokens[2].type, TokenType::LBRACE);
    EXPECT_EQ(tokens[3].type, TokenType::INT);
    EXPECT_EQ(tokens[4].type, TokenType::ID);
    EXPECT_EQ(tokens[4].lexeme, "x");
    EXPECT_EQ(tokens[5].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokens[6].type, TokenType::RBRACE);
}

TEST_F(LexerTest, TokenizeNewExpression) {
    std::vector<Token> tokens = getAllTokens("new Point()");
    ASSERT_GE(tokens.size(), 5);
    EXPECT_EQ(tokens[0].type, TokenType::NEW);
    EXPECT_EQ(tokens[1].type, TokenType::ID);
    EXPECT_EQ(tokens[1].lexeme, "Point");
    EXPECT_EQ(tokens[2].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[3].type, TokenType::RPAREN);
}

TEST_F(LexerTest, TokenizeMethodCallWithArgs) {
    std::vector<Token> tokens = getAllTokens("obj.method(a, b)");
    ASSERT_GE(tokens.size(), 8);
    EXPECT_EQ(tokens[0].type, TokenType::ID);
    EXPECT_EQ(tokens[1].type, TokenType::DOT);
    EXPECT_EQ(tokens[2].type, TokenType::ID);
    EXPECT_EQ(tokens[3].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[4].type, TokenType::ID);
    EXPECT_EQ(tokens[5].type, TokenType::COMMA);
    EXPECT_EQ(tokens[6].type, TokenType::ID);
    EXPECT_EQ(tokens[7].type, TokenType::RPAREN);
}

TEST_F(LexerTest, TokenizeArrayAccess) {
    std::vector<Token> tokens = getAllTokens("arr[0]");
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::ID);
    EXPECT_EQ(tokens[1].type, TokenType::LBRACKET);
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[3].type, TokenType::RBRACKET);
}
