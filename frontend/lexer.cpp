#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& input) : input(input), pos(0) {}

char Lexer::peek() {
    if (pos >= input.size()) {
        return '\0';
    }
    return input[pos];
}

char Lexer::get() {
    if (pos >= input.size()) {
        return '\0';
    }
    return input[pos++];
}

void Lexer::skipWhitespace() {
    while (std::isspace(peek())) {
        get();
    }
}

Token Lexer::nextToken() {
    skipWhitespace();
    char c = peek();

    if (c == '\0') {
        return {TokenType::END, ""};
    }

    if (std::isalpha(c)) {
        std::string value;
        while (std::isalnum(peek())) {
            value += get();
        }

        if (value == "declare") {
            return {TokenType::DECLARE, value};
        }
        if (value == "int") {
            return {TokenType::INT, value};
        }
        if (value == "if") {
            return {TokenType::IF, value};
        }
        if (value == "else") {
            return {TokenType::ELSE, value};
        }
        if (value == "print") {
            return {TokenType::PRINT, value};
        }

        return {TokenType::ID, value};
    }

    if (std::isdigit(c)) {
        std::string value;
        while (std::isdigit(peek())) {
            value += get();
        }
        return {TokenType::NUMBER, value};
    }

    if (c == '=') {
        get();
        if (peek() == '=') {
            get();
            return {TokenType::EQUAL, "=="};
        }
        return {TokenType::ASSIGN, "="};
    }

    if (c == ':') {
        get();
        return {TokenType::COLON, ":"};
    }
    if (c == ';') {
        get();
        return {TokenType::SEMICOLON, ";"};
    }
    if (c == '(') {
        get();
        return {TokenType::LPAREN, "("};
    }
    if (c == ')') {
        get();
        return {TokenType::RPAREN, ")"};
    }
    if (c == '{') {
        get();
        return {TokenType::LBRACE, "{"};
    }
    if (c == '}') {
        get();
        return {TokenType::RBRACE, "}"};
    }

    throw std::runtime_error("Unknown character");
}