#pragma once
#include <string>
#include <unordered_map>

enum class TokenType {
    DECLARE,
    INT,
    IF,
    ELSE,
    PRINT,

    ID,
    NUMBER,

    PLUS,
    MINUS,
    MUL,
    DIV,

    EQUAL,
    ASSIGN,

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

    Token(TokenType type, std::string lexeme)
        : type(type), lexeme(std::move(lexeme)) {}
};

inline const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"declare", TokenType::DECLARE},
    {"int", TokenType::INT},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"print", TokenType::PRINT}
};

inline const std::unordered_map<char, TokenType> SINGLE_TOKENS = {
    {':', TokenType::COLON},
    {';', TokenType::SEMICOLON},
    {'(', TokenType::LPAREN},
    {')', TokenType::RPAREN},
    {'{', TokenType::LBRACE},
    {'}', TokenType::RBRACE},
    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'*', TokenType::MUL},
    {'/', TokenType::DIV}
};