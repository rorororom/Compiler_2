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

Parser::Parser(Lexer lexer) : lexer(lexer) {
    current = this->lexer.nextToken();
}

void Parser::consume(TokenType type) {
    if (current.type != type) {
        throw std::runtime_error("Unexpected token");
    }
    current = lexer.nextToken();
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (current.type != TokenType::END) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

std::unique_ptr<Node> Parser::parseStatement() {
    if (current.type == TokenType::DECLARE) {
        consume(TokenType::DECLARE);
        std::string name = current.lexeme;

        consume(TokenType::ID);
        consume(TokenType::COLON);
        consume(TokenType::INT);
        consume(TokenType::SEMICOLON);

        return std::make_unique<DeclareStmt>(name);
    }

    if (current.type == TokenType::IF) {

        consume(TokenType::IF);
        consume(TokenType::LPAREN);
        auto cond = parseExpression();
        consume(TokenType::RPAREN);
        consume(TokenType::LBRACE);
        
        auto ifStmt = std::make_unique<IfStmt>(std::move(cond));
        
        while (current.type != TokenType::RBRACE && current.type != TokenType::END) {
            ifStmt->thenBranch.push_back(parseStatement());
        }
        
        consume(TokenType::RBRACE);
        
        if (current.type == TokenType::ELSE) {
            consume(TokenType::ELSE);
            consume(TokenType::LBRACE);
            
            while (current.type != TokenType::RBRACE && current.type != TokenType::END) {
                ifStmt->elseBranch.push_back(parseStatement());
            }
            
            consume(TokenType::RBRACE);
        }
        
        return ifStmt;
    }

    if (current.type == TokenType::ID) {
        std::string name = current.lexeme;
        consume(TokenType::ID);
        consume(TokenType::ASSIGN);

        auto expr = parseExpression();
        consume(TokenType::SEMICOLON);

        return std::make_unique<AssignStmt>(name, std::move(expr));
    }

    if (current.type == TokenType::PRINT) {
        consume(TokenType::PRINT);
        consume(TokenType::LPAREN);
        auto expr = parseExpression();

        consume(TokenType::RPAREN);
        consume(TokenType::SEMICOLON);

        return std::make_unique<PrintStmt>(std::move(expr));
    }

    throw std::runtime_error("Unknown statement");
}

std::unique_ptr<Expression> Parser::parseExpression() {
    auto left = parsePrimary();
    
    while (current.type == TokenType::EQUAL) {
        consume(TokenType::EQUAL);
        auto right = parsePrimary();
        left = std::make_unique<EqualExpr>(std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    if (current.type == TokenType::NUMBER) {
        int val = stoi(current.lexeme);
        consume(TokenType::NUMBER);
        return std::make_unique<NumberExpr>(val);
    }

    if (current.type == TokenType::ID) {
        std::string name = current.lexeme;
        consume(TokenType::ID);
        return std::make_unique<VariableExpr>(name);
    }

    throw std::runtime_error("Invalid expression");
}
