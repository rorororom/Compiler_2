#pragma once
#include "token.h"
#include <string>

class Lexer {
public:
    explicit Lexer(const std::string& input);

    Token nextToken();

private:
    std::string input;
    size_t pos;

    char peek();
    char get();
    void skipWhitespace();
};