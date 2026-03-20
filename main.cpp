#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "interpreter_visitor.h"
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
        std::cout << "AST written to ast_output.txt" << std::endl;

        std::cout << "\nProgram output:" << std::endl;
        InterpreterVisitor interpreter;
        program->accept(&interpreter);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
