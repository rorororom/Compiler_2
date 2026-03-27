#pragma once
#include "token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos;

    char peek();
    char get();
    void skipWhitespace();
    Token nextToken();
};