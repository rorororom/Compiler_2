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
    while (true) {
        while (std::isspace(peek())) get();
        if (peek() == '/' && pos + 1 < input.size() && input[pos + 1] == '/') {
            while (peek() != '\n' && peek() != '\0') get();
        } else {
            break;
        }
    }
}

Token Lexer::nextToken() {
    skipWhitespace();
    char c = peek();

    if (c == '\0') {
        return Token(TokenType::END, "");
    }

    if (std::isalpha(c)) {
        std::string value;
        while (std::isalnum(peek())) {
            value += get();
        }
        
        auto it = KEYWORDS.find(value);
        if (it != KEYWORDS.end()) {
            return {it->second, value};
        }
        
        return Token(TokenType::ID, value);
    }

    if (std::isdigit(c)) {
        std::string value;
        while (std::isdigit(peek())) {
            value += get();
        }
        return Token(TokenType::NUMBER, value);
    }

    if (c == '=') {
        get();
        if (peek() == '=') {
            get();
            return Token(TokenType::EQUAL, "==");
        }
        return Token(TokenType::ASSIGN, "=");
    }

    auto it = SINGLE_TOKENS.find(c);
    if (it != SINGLE_TOKENS.end()) {
        get();
        return {it->second, std::string(1, c)};
    }

    throw std::runtime_error("Unknown character");
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        
        if (token.type == TokenType::END) {
            break;
        }
    }
    
    return tokens;
}