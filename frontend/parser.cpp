#include "parser.h"
#include "context.h"
#include <stdexcept>
#include <iostream>

class NumberExpr : public Expression {
    int value;
public:
    NumberExpr(int v) : value(v) {}
    int evaluate(Context&) override { return value; }
};

class VariableExpr : public Expression {
    std::string name;
public:
    VariableExpr(std::string n) : name(n) {}
    int evaluate(Context& ctx) override;
};

class EqualExpr : public Expression {
    std::unique_ptr<Expression> left, right;
public:
    EqualExpr(std::unique_ptr<Expression> l,
              std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}

    int evaluate(Context& ctx) override {
        return left->evaluate(ctx) == right->evaluate(ctx);
    }
};

class DeclareStmt : public Node {
    std::string name;
public:
    DeclareStmt(std::string n) : name(n) {}
    void execute(Context& ctx) override;
};

class AssignStmt : public Node {
    std::string name;
    std::unique_ptr<Expression> expr;
public:
    AssignStmt(std::string n, std::unique_ptr<Expression> e)
        : name(n), expr(std::move(e)) {}
    void execute(Context& ctx) override;
};

class PrintStmt : public Node {
    std::unique_ptr<Expression> expr;
public:
    PrintStmt(std::unique_ptr<Expression> e)
        : expr(std::move(e)) {}
    void execute(Context& ctx) override;
};

class IfStmt : public Node {
    std::unique_ptr<Expression> cond;
public:
    std::vector<std::unique_ptr<Node>> thenBranch;
    std::vector<std::unique_ptr<Node>> elseBranch;
    
    IfStmt(std::unique_ptr<Expression> c) : cond(std::move(c)) {}
    void execute(Context& ctx) override;
};

class BinaryExpr : public Expression {
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    TokenType op;
    
public:
    BinaryExpr(std::unique_ptr<Expression> l, 
               TokenType op,
               std::unique_ptr<Expression> r)
        : left(std::move(l)), op(op), right(std::move(r)) {}
    
    int evaluate(Context& ctx) override {
        int leftVal = left->evaluate(ctx);
        int rightVal = right->evaluate(ctx);
        
        switch (op) {
            case TokenType::PLUS:  return leftVal + rightVal;
            case TokenType::MINUS: return leftVal - rightVal;
            case TokenType::MUL:  return leftVal * rightVal;
            case TokenType::DIV: 
                if (rightVal == 0) {
                    throw std::runtime_error("Division by zero");
                }
                return leftVal / rightVal;
            default:
                throw std::runtime_error("Unknown binary operator");
        }
    }
};

int VariableExpr::evaluate(Context& ctx) {
    return ctx.get(name);
}

void DeclareStmt::execute(Context& ctx) {
    ctx.declare(name);
}

void AssignStmt::execute(Context& ctx) {
    ctx.assign(name, expr->evaluate(ctx));
}

void PrintStmt::execute(Context& ctx) {
    ctx.print(expr->evaluate(ctx));
}

void IfStmt::execute(Context& ctx) {
    if (cond->evaluate(ctx)) {
        for (auto& stmt : thenBranch) {
            stmt->execute(ctx);
        }
    } else {
        for (auto& stmt : elseBranch) {
            stmt->execute(ctx);
        }
    }
}

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
            ifStmt->thenBranch.push_back(parseStatement());
        }
        
        consume(TokenType::RBRACE);

        if (match(TokenType::ELSE)) {
            consume(TokenType::LBRACE);
            
            while (!check(TokenType::RBRACE) && !isAtEnd()) {
                ifStmt->elseBranch.push_back(parseStatement());
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
    // NUMBER literal
    if (check(TokenType::NUMBER)) {
        int val = std::stoi(current().lexeme);
        advance();
        return std::make_unique<NumberExpr>(val);
    }

    // VARIABLE reference
    if (check(TokenType::ID)) {
        std::string name = current().lexeme;
        advance();
        return std::make_unique<VariableExpr>(name);
    }

    // Parenthesized expression
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN);
        return expr;
    }

    throw std::runtime_error("Invalid expression at position " + 
                            std::to_string(position) +
                            ": unexpected token '" + current().lexeme + "'");
}