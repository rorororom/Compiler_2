#include "print_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
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
    for (size_t i = 0; i < indent; ++i) {
        out << "  ";
    }
}

std::string PrintVisitor::opToString(TokenType op) {
    auto it = OP_TO_STRING.find(op);
    return it != OP_TO_STRING.end() ? it->second : "?";
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

void PrintVisitor::visit(ClassDecl* node) {
    printIndent();
    out << "Class: " << node->getName() << " {\n";
    indent++;
    for (auto& f : node->getFields())  f->accept(this);
    for (auto& m : node->getMethods()) m->accept(this);
    indent--;
    printIndent();
    out << "}\n";
}

void PrintVisitor::visit(FieldDecl* node) {
    printIndent();
    out << "Field: " << node->getName()
        << " : " << node->getType().typeName
        << (node->getType().isArray ? "[]" : "") << "\n";
}

void PrintVisitor::visit(MethodDecl* node) {
    printIndent();
    out << "Method: " << node->getName() << "(";
    bool first = true;
    for (auto& p : node->getParams()) {
        if (!first) out << ", ";
        out << p.type.typeName << (p.type.isArray ? "[]" : "") << " " << p.name;
        first = false;
    }
    out << ") : " << node->getReturnType().typeName
        << (node->getReturnType().isArray ? "[]" : "") << " {\n";
    indent++;
    for (auto& s : node->getBody()) s->accept(this);
    indent--;
    printIndent();
    out << "}\n";
}

void PrintVisitor::visit(ReturnStmt* node) {
    printIndent();
    out << "Return: ";
    if (node->getExpression()) node->getExpression()->accept(this);
    out << "\n";
}

void PrintVisitor::visit(VarDeclStmt* node) {
    printIndent();
    out << "VarDecl: " << node->getName()
        << " : " << node->getType().typeName
        << (node->getType().isArray ? "[]" : "");
    if (node->getInitializer()) {
        out << " = ";
        node->getInitializer()->accept(this);
    }
    out << "\n";
}

void PrintVisitor::visit(NewObjectExpr* node) {
    out << "new " << node->getClassName() << "(";
    bool first = true;
    for (auto& a : node->getArgs()) {
        if (!first) out << ", ";
        a->accept(this);
        first = false;
    }
    out << ")";
}

void PrintVisitor::visit(NewArrayExpr* node) {
    out << "new " << node->getElemType().typeName << "[";
    node->getSize()->accept(this);
    out << "]";
}

void PrintVisitor::visit(MethodCallExpr* node) {
    node->getObject()->accept(this);
    out << "." << node->getMethodName() << "(";
    bool first = true;
    for (auto& a : node->getArgs()) {
        if (!first) out << ", ";
        a->accept(this);
        first = false;
    }
    out << ")";
}

void PrintVisitor::visit(FieldAccessExpr* node) {
    node->getObject()->accept(this);
    out << "." << node->getFieldName();
}

void PrintVisitor::visit(ThisExpr* /*node*/) {
    out << "this";
}

void PrintVisitor::visit(ArrayAccessExpr* node) {
    node->getArray()->accept(this);
    out << "[";
    node->getIndex()->accept(this);
    out << "]";
}

void PrintVisitor::visit(ArrayLengthExpr* node) {
    node->getArray()->accept(this);
    out << ".length";
}
