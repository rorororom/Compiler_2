#pragma once
#include "expression.h"
#include "lexer.h"
#include "program.h"
#include <memory>

class Parser {
public:
    explicit Parser(Lexer lexer);

    std::unique_ptr<Program> parseProgram();

private:
    Lexer lexer;
    Token current;

    void consume(TokenType type);
    std::unique_ptr<Node> parseStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parsePrimary();
};