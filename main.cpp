#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "interpreter_visitor.h"
#include "scope_visitor.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string code = ss.str();

    try {
        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        auto program = parser.parseProgram();

        PrintVisitor printVisitor("ast_output.txt");
        program->accept(&printVisitor);
        std::cout << "AST written to ast_output.txt\n";

        std::cout << "\n=== Semantic Analysis ===\n";
        ScopeVisitor scopeVisitor;

        scopeVisitor.collectClasses(program.get());

        program->accept(&scopeVisitor);

        std::cout << "\n--- Scope Tree ---\n";
        scopeVisitor.getScopeTree().dump(std::cout);

        std::ofstream scopeOut("scope_tree_output.txt");
        if (scopeOut.is_open()) {
            scopeVisitor.getScopeTree().dump(scopeOut);
            std::cout << "\nScope tree written to scope_tree_output.txt\n";
        }

        std::cout << "\n--- Symbol Table (classes) ---\n";
        for (auto& [name, cls] : scopeVisitor.getSymbolTable().allClasses()) {
            std::cout << "class " << name << " {\n";
            std::cout << "  fields (" << cls.fieldCount() << "):\n";
            for (auto& f : cls.fields) {
                std::cout << "    " << f.name << " : " << f.type.toString() << "\n";
            }
            std::cout << "  methods:\n";
            for (auto& [mname, m] : cls.methods) {
                std::cout << "    " << (m.isConstructor ? "[ctor] " : "")
                          << m.returnType.toString() << " " << m.name << "(";
                bool first = true;
                for (auto& p : m.params) {
                    if (!first) std::cout << ", ";
                    std::cout << p.type.toString() << " " << p.name;
                    first = false;
                }
                std::cout << ")\n";
            }
            std::cout << "}\n";
        }

        std::cout << "\nSemantic analysis passed.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
