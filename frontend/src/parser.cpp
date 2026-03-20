#include "parser.h"
#include "statements.h"
#include "expressions.h"
#include "program.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), position(0) {} 

const Token& Parser::current() const {
    if (position >= tokens.size()) {
        return tokens.back();
    }
    return tokens[position];
}

const Token& Parser::peek(int offset) const {
    size_t pos = position + offset;
    if (pos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos];
}

const Token& Parser::previous() const {
    if (position == 0) {
        throw std::runtime_error("No previous token");
    }
    return tokens[position - 1];
}

void Parser::advance() {
    if (!isAtEnd()) {
        position++;
    }
}

bool Parser::isAtEnd() const {
    return current().type == TokenType::END;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type) {
    if (current().type != type) {
        throw std::runtime_error("Unexpected token at position " + 
                                std::to_string(position) + 
                                ": expected token type " + std::to_string(static_cast<int>(type)) +
                                ", got '" + current().lexeme + "'");
    }
    advance();
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

std::unique_ptr<Node> Parser::parseStatement() {
    if (match(TokenType::DECLARE)) {
        std::string name = current().lexeme;
        consume(TokenType::ID);
        consume(TokenType::COLON);
        consume(TokenType::INT);
        consume(TokenType::SEMICOLON);
        return std::make_unique<DeclareStmt>(name);
    }

    if (match(TokenType::IF)) {
        consume(TokenType::LPAREN);
        auto cond = parseExpression();
        consume(TokenType::RPAREN);
        consume(TokenType::LBRACE);
        
        auto ifStmt = std::make_unique<IfStmt>(std::move(cond));
    
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            ifStmt->addThenStatement(parseStatement());
        }
        
        consume(TokenType::RBRACE);

        if (match(TokenType::ELSE)) {
            consume(TokenType::LBRACE);
            
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                ifStmt->addElseStatement(parseStatement());
            }
            
            consume(TokenType::RBRACE);
        }
        
        return ifStmt;
    }

    if (match(TokenType::PRINT)) {
        consume(TokenType::LPAREN);
        auto expr = parseExpression();
        consume(TokenType::RPAREN);
        consume(TokenType::SEMICOLON);
        return std::make_unique<PrintStmt>(std::move(expr));
    }

    if (check(TokenType::ID)) {
        std::string name = current().lexeme;
        advance();
        consume(TokenType::ASSIGN);
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON);
        return std::make_unique<AssignStmt>(name, std::move(expr));
    }

    throw std::runtime_error("Unknown statement at position " + 
                            std::to_string(position) + 
                            ": unexpected token '" + current().lexeme + "'");
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto left = parseAddition();
    
    while (match(TokenType::EQUAL)) {
        auto right = parseAddition();
        left = std::make_unique<EqualExpr>(std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseAddition() {
    auto left = parseMultiplication();
    
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        TokenType op = current().type;
        advance();
        auto right = parseMultiplication();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseMultiplication() {
    auto left = parsePrimary();
    
    while (check(TokenType::MUL) || check(TokenType::DIV)) {
        TokenType op = current().type;
        advance();
        auto right = parsePrimary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        int val = std::stoi(current().lexeme);
        advance();
        return std::make_unique<NumberExpr>(val);
    }

    if (check(TokenType::ID)) {
        std::string name = current().lexeme;
        advance();
        return std::make_unique<VariableExpr>(name);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN);
        return expr;
    }

    throw std::runtime_error("Invalid expression at position " + 
                            std::to_string(position) +
                            ": unexpected token '" + current().lexeme + "'");
}