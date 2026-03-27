#pragma once
#include "expression.h"
#include "token.h"
#include "program.h"
#include "oop_nodes.h"
#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<Program> parseProgram();

private:
    std::vector<Token> tokens;
    size_t position;
    
    const Token& current() const;
    const Token& peek(int offset = 0) const;
    const Token& previous() const;
    
    void advance();
    void consume(TokenType type);
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;

    TypeAnnotation parseTypeAnnotation();

    std::unique_ptr<Node>       parseStatement();
    std::unique_ptr<Node>       parseVarDeclStmt();
    std::unique_ptr<Node>       parseClassDecl();
    std::unique_ptr<MethodDecl> parseMethodDecl(TypeAnnotation retType,
                                                 std::string name);

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseAddition();
    std::unique_ptr<Expression> parseMultiplication();
    std::unique_ptr<Expression> parsePostfix();
    std::unique_ptr<Expression> parsePrimary();
};
