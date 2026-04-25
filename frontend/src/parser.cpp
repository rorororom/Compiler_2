#include "parser.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
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

TypeInfo Parser::parseTypeAnnotation() {
    TypeInfo ann;
    if (check(TokenType::INT)) {
        advance();
        if (check(TokenType::LBRACKET)) {
            advance();
            consume(TokenType::RBRACKET);
            ann = TypeInfo::makeArrayInt();
        } else {
            ann = TypeInfo::makeInt();
        }
    } else if (check(TokenType::VOID)) {
        advance();
        ann = TypeInfo::makeVoid();
    } else if (check(TokenType::ID)) {
        std::string name = current().lexeme;
        advance();
        if (check(TokenType::LBRACKET)) {
            advance();
            consume(TokenType::RBRACKET);
            ann = TypeInfo::makeArrayClass(name);
        } else {
            ann = TypeInfo::makeClass(name);
        }
    } else {
        throw std::runtime_error(
            "Expected type name at position " + std::to_string(position) +
            ", got '" + current().lexeme + "'");
    }
    return ann;
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

std::unique_ptr<Node> Parser::parseStatement() {
    if (match(TokenType::CLASS)) {
        return parseClassDecl();
    }

    if (match(TokenType::DECLARE)) {
        std::string name = current().lexeme;
        consume(TokenType::ID);
        consume(TokenType::COLON);
        consume(TokenType::INT);
        consume(TokenType::SEMICOLON);
        return std::make_unique<DeclareStmt>(name);
    }

    if (match(TokenType::RETURN)) {
        if (check(TokenType::SEMICOLON)) {
            consume(TokenType::SEMICOLON);
            return std::make_unique<ReturnStmt>();
        }
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON);
        return std::make_unique<ReturnStmt>(std::move(expr));
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

    if ((check(TokenType::INT) || check(TokenType::VOID) ||
         (check(TokenType::ID) && peek(1).type == TokenType::ID))) {
        return parseVarDeclStmt();
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

std::unique_ptr<Node> Parser::parseVarDeclStmt() {
    TypeInfo type = parseTypeAnnotation();
    std::string name = current().lexeme;
    consume(TokenType::ID);

    std::unique_ptr<Expression> init;
    if (match(TokenType::ASSIGN)) {
        init = parseExpression();
    }
    consume(TokenType::SEMICOLON);
    return std::make_unique<VarDeclStmt>(std::move(type), std::move(name), std::move(init));
}

std::unique_ptr<Node> Parser::parseClassDecl() {
    std::string className = current().lexeme;
    consume(TokenType::ID);
    consume(TokenType::LBRACE);

    auto cls = std::make_unique<ClassDecl>(className);

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        TypeInfo type = parseTypeAnnotation();
        std::string memberName = current().lexeme;
        consume(TokenType::ID);

        if (check(TokenType::LPAREN)) {
            cls->addMethod(parseMethodDecl(std::move(type), std::move(memberName)));
        } else {
            consume(TokenType::SEMICOLON);
            cls->addField(std::make_unique<FieldDecl>(std::move(type), std::move(memberName)));
        }
    }

    consume(TokenType::RBRACE);
    return cls;
}

std::unique_ptr<MethodDecl> Parser::parseMethodDecl(TypeInfo retType,
                                                     std::string name) {
    consume(TokenType::LPAREN);

    MethodSymbol sym(name, std::move(retType), /*ownerClass=*/"");
    while (!check(TokenType::RPAREN) && !isAtEnd()) {
        TypeInfo ptype = parseTypeAnnotation();
        std::string pname = current().lexeme;
        consume(TokenType::ID);
        sym.addParam(VariableSymbol(std::move(pname), std::move(ptype),
                                    VariableSymbol::StorageKind::PARAM));
        if (!check(TokenType::RPAREN)) {
            consume(TokenType::COMMA);
        }
    }
    consume(TokenType::RPAREN);
    consume(TokenType::LBRACE);

    auto method = std::make_unique<MethodDecl>(std::move(sym));

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        method->addStatement(parseStatement());
    }
    consume(TokenType::RBRACE);

    return method;
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
    auto left = parsePostfix();

    while (check(TokenType::MUL) || check(TokenType::DIV)) {
        TokenType op = current().type;
        advance();
        auto right = parsePostfix();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<Expression> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (check(TokenType::DOT)) {
            advance();
            std::string memberName = current().lexeme;
            consume(TokenType::ID);

            if (check(TokenType::LPAREN)) {
                advance();
                auto call = std::make_unique<MethodCallExpr>(std::move(expr), memberName);
                while (!check(TokenType::RPAREN) && !isAtEnd()) {
                    call->addArg(parseExpression());
                    if (!check(TokenType::RPAREN)) consume(TokenType::COMMA);
                }
                consume(TokenType::RPAREN);
                expr = std::move(call);
            } else {
                if (memberName == "length") {
                    expr = std::make_unique<ArrayLengthExpr>(std::move(expr));
                } else {
                    expr = std::make_unique<FieldAccessExpr>(std::move(expr), memberName);
                }
            }
        } else if (check(TokenType::LBRACKET)) {
            advance();
            auto index = parseExpression();
            consume(TokenType::RBRACKET);
            expr = std::make_unique<ArrayAccessExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        int val = std::stoi(current().lexeme);
        advance();
        return std::make_unique<NumberExpr>(val);
    }

    if (match(TokenType::THIS)) {
        return std::make_unique<ThisExpr>();
    }

    if (match(TokenType::NEW)) {
        TypeInfo elemType = parseTypeAnnotation();

        if (check(TokenType::LBRACKET)) {
            advance();
            auto size = parseExpression();
            consume(TokenType::RBRACKET);
            return std::make_unique<NewArrayExpr>(std::move(elemType), std::move(size));
        } else {
            consume(TokenType::LPAREN);
            auto obj = std::make_unique<NewObjectExpr>(elemType.className);
            while (!check(TokenType::RPAREN) && !isAtEnd()) {
                obj->addArg(parseExpression());
                if (!check(TokenType::RPAREN)) consume(TokenType::COMMA);
            }
            consume(TokenType::RPAREN);
            return obj;
        }
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
