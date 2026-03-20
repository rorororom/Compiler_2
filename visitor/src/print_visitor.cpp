#include "print_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "program.h"
#include <iostream>

PrintVisitor::PrintVisitor(const std::string& filename) : out(filename) {
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
}

PrintVisitor::~PrintVisitor() {
    if (out.is_open()) {
        out.close();
    }
}

void PrintVisitor::printIndent() {
    for (int i = 0; i < indent; ++i) {
        out << "  ";
    }
}

std::string PrintVisitor::opToString(TokenType op) {
    switch (op) {
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::MUL: return "*";
        case TokenType::DIV: return "/";
        case TokenType::EQUAL: return "==";
        default: return "?";
    }
}

void PrintVisitor::visit(Program* node) {
    out << "Program {" << std::endl;
    indent++;
    for (auto& stmt : node->getStatements()) {
        stmt->accept(this);
    }
    indent--;
    out << "}" << std::endl;
}

void PrintVisitor::visit(DeclareStmt* node) {
    printIndent();
    out << "Declare: " << node->getName() << " : int" << std::endl;
}

void PrintVisitor::visit(AssignStmt* node) {
    printIndent();
    out << "Assign: " << node->getName() << " = ";
    node->getExpression()->accept(this);
    out << std::endl;
}

void PrintVisitor::visit(PrintStmt* node) {
    printIndent();
    out << "Print: ";
    node->getExpression()->accept(this);
    out << std::endl;
}

void PrintVisitor::visit(IfStmt* node) {
    printIndent();
    out << "If (" ;
    node->getCondition()->accept(this);
    out << ") {" << std::endl;
    
    indent++;
    for (auto& stmt : node->getThenBranch()) {
        stmt->accept(this);
    }
    indent--;
    
    if (!node->getElseBranch().empty()) {
        printIndent();
        out << "} else {" << std::endl;
        indent++;
        for (auto& stmt : node->getElseBranch()) {
            stmt->accept(this);
        }
        indent--;
    }
    
    printIndent();
    out << "}" << std::endl;
}

void PrintVisitor::visit(NumberExpr* node) {
    out << node->getValue();
}

void PrintVisitor::visit(VariableExpr* node) {
    out << node->getName();
}

void PrintVisitor::visit(BinaryExpr* node) {
    out << "(";
    node->getLeft()->accept(this);
    out << " " << opToString(node->getOperator()) << " ";
    node->getRight()->accept(this);
    out << ")";
}

void PrintVisitor::visit(EqualExpr* node) {
    out << "(";
    node->getLeft()->accept(this);
    out << " == ";
    node->getRight()->accept(this);
    out << ")";
}
