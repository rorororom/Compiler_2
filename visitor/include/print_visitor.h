#pragma once
#include "visitor.h"
#include "token.h"
#include <fstream>
#include <string>
#include <unordered_map>

inline const std::unordered_map<TokenType, std::string> OP_TO_STRING = {
    {TokenType::PLUS,  "+"},
    {TokenType::MINUS, "-"},
    {TokenType::MUL,   "*"},
    {TokenType::DIV,   "/"},
    {TokenType::EQUAL, "=="},
};

class PrintVisitor : public Visitor {
    std::ofstream out;
    size_t indent = 0;

    void printIndent();
    std::string opToString(TokenType op);

public:
    explicit PrintVisitor(const std::string& filename);
    ~PrintVisitor();
};
