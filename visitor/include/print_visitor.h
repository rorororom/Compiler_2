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
    
    void visit(Program* node) override;
    void visit(DeclareStmt* node) override;
    void visit(AssignStmt* node) override;
    void visit(PrintStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(NumberExpr* node) override;
    void visit(VariableExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(EqualExpr* node) override;
};
