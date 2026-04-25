#include "print_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
#include "program.h"
#include <iostream>

PrintVisitor::PrintVisitor(const std::string& filename) : out(filename) {
    if (!out.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    registerHandler<Program>([this](Program* node) {
        out << "Program {" << std::endl;
        indent++;
        for (auto& stmt : node->getStatements())
            stmt->accept(this);
        indent--;
        out << "}" << std::endl;
    });

    registerHandler<DeclareStmt>([this](DeclareStmt* node) {
        printIndent();
        out << "Declare: " << node->getName() << " : int" << std::endl;
    });

    registerHandler<AssignStmt>([this](AssignStmt* node) {
        printIndent();
        out << "Assign: " << node->getName() << " = ";
        node->getExpression()->accept(this);
        out << std::endl;
    });

    registerHandler<PrintStmt>([this](PrintStmt* node) {
        printIndent();
        out << "Print: ";
        node->getExpression()->accept(this);
        out << std::endl;
    });

    registerHandler<IfStmt>([this](IfStmt* node) {
        printIndent();
        out << "If (";
        node->getCondition()->accept(this);
        out << ") {" << std::endl;

        indent++;
        for (auto& stmt : node->getThenBranch())
            stmt->accept(this);
        indent--;

        if (!node->getElseBranch().empty()) {
            printIndent();
            out << "} else {" << std::endl;
            indent++;
            for (auto& stmt : node->getElseBranch())
                stmt->accept(this);
            indent--;
        }

        printIndent();
        out << "}" << std::endl;
    });

    registerHandler<NumberExpr>([this](NumberExpr* node) {
        out << node->getValue();
    });

    registerHandler<VariableExpr>([this](VariableExpr* node) {
        out << node->getName();
    });

    registerHandler<BinaryExpr>([this](BinaryExpr* node) {
        out << "(";
        node->getLeft()->accept(this);
        out << " " << opToString(node->getOperator()) << " ";
        node->getRight()->accept(this);
        out << ")";
    });

    registerHandler<EqualExpr>([this](EqualExpr* node) {
        out << "(";
        node->getLeft()->accept(this);
        out << " == ";
        node->getRight()->accept(this);
        out << ")";
    });

    registerHandler<ClassDecl>([this](ClassDecl* node) {
        printIndent();
        out << "Class: " << node->getName() << " {\n";
        indent++;
        for (auto& f : node->getFields())  f->accept(this);
        for (auto& m : node->getMethods()) m->accept(this);
        indent--;
        printIndent();
        out << "}\n";
    });

    registerHandler<FieldDecl>([this](FieldDecl* node) {
        printIndent();
        out << "Field: " << node->getName()
            << " : " << node->getType().toString() << "\n";
    });

    registerHandler<MethodDecl>([this](MethodDecl* node) {
        printIndent();
        out << "Method: " << node->getName() << "(";
        bool first = true;
        for (const auto& p : node->getParams()) {
            if (!first) out << ", ";
            out << p.type.toString() << " " << p.name;
            first = false;
        }
        out << ") : " << node->getReturnType().toString() << " {\n";
        indent++;
        for (auto& s : node->getBody()) s->accept(this);
        indent--;
        printIndent();
        out << "}\n";
    });

    registerHandler<ReturnStmt>([this](ReturnStmt* node) {
        printIndent();
        out << "Return: ";
        if (node->getExpression()) node->getExpression()->accept(this);
        out << "\n";
    });

    registerHandler<VarDeclStmt>([this](VarDeclStmt* node) {
        printIndent();
        out << "VarDecl: " << node->getName()
            << " : " << node->getType().toString();
        if (node->getInitializer()) {
            out << " = ";
            node->getInitializer()->accept(this);
        }
        out << "\n";
    });

    registerHandler<NewObjectExpr>([this](NewObjectExpr* node) {
        out << "new " << node->getClassName() << "(";
        bool first = true;
        for (auto& a : node->getArgs()) {
            if (!first) out << ", ";
            a->accept(this);
            first = false;
        }
        out << ")";
    });

    registerHandler<NewArrayExpr>([this](NewArrayExpr* node) {
        out << "new " << node->getElemType().toString() << "[";
        node->getSize()->accept(this);
        out << "]";
    });

    registerHandler<MethodCallExpr>([this](MethodCallExpr* node) {
        node->getObject()->accept(this);
        out << "." << node->getMethodName() << "(";
        bool first = true;
        for (auto& a : node->getArgs()) {
            if (!first) out << ", ";
            a->accept(this);
            first = false;
        }
        out << ")";
    });

    registerHandler<FieldAccessExpr>([this](FieldAccessExpr* node) {
        node->getObject()->accept(this);
        out << "." << node->getFieldName();
    });

    registerHandler<ThisExpr>([this](ThisExpr*) {
        out << "this";
    });

    registerHandler<ArrayAccessExpr>([this](ArrayAccessExpr* node) {
        node->getArray()->accept(this);
        out << "[";
        node->getIndex()->accept(this);
        out << "]";
    });

    registerHandler<ArrayLengthExpr>([this](ArrayLengthExpr* node) {
        node->getArray()->accept(this);
        out << ".length";
    });
}

PrintVisitor::~PrintVisitor() {
    if (out.is_open())
        out.close();
}

void PrintVisitor::printIndent() {
    for (size_t i = 0; i < indent; ++i)
        out << "  ";
}

std::string PrintVisitor::opToString(TokenType op) {
    auto it = OP_TO_STRING.find(op);
    return it != OP_TO_STRING.end() ? it->second : "?";
}
