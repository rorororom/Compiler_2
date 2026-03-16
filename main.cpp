#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "interpreter_visitor.h"
#include <iostream>

int main() {
    std::string code = R"(
        declare x: int;
        declare y: int;
        
        x = 10;
        y = 20;
        
        if (x + y == 30) {
            print(999);
        } else {
            print(111);
        }
    )";

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
        interpreter.visit(program.get());
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
