#pragma once
#include <string>
#include <unordered_map>

enum class TokenType {
    DECLARE,
    INT,
    IF,
    ELSE,
    PRINT,

    CLASS,
    NEW,
    THIS,
    RETURN,
    VOID,

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
    COMMA,
    DOT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,

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
    {"int",     TokenType::INT},
    {"if",      TokenType::IF},
    {"else",    TokenType::ELSE},
    {"print",   TokenType::PRINT},
    {"class",   TokenType::CLASS},
    {"new",     TokenType::NEW},
    {"this",    TokenType::THIS},
    {"return",  TokenType::RETURN},
    {"void",    TokenType::VOID}
};

inline const std::unordered_map<char, TokenType> SINGLE_TOKENS = {
    {':', TokenType::COLON},
    {';', TokenType::SEMICOLON},
    {',', TokenType::COMMA},
    {'.', TokenType::DOT},
    {'(', TokenType::LPAREN},
    {')', TokenType::RPAREN},
    {'{', TokenType::LBRACE},
    {'}', TokenType::RBRACE},
    {'[', TokenType::LBRACKET},
    {']', TokenType::RBRACKET},
    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'*', TokenType::MUL},
    {'/', TokenType::DIV}
};