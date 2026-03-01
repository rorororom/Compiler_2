#pragma once
#include <string>

enum class TokenType {
    DECLARE,
    INT,
    IF,
    ELSE,
    PRINT,
    ID,
    NUMBER,
    ASSIGN,
    EQUAL,
    COLON,
    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    END
};

struct Token {
    TokenType type;
    std::string lexeme;
};