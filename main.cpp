#include "lexer.h"
#include "parser.h"
#include "context.h"
#include <iostream>

int main() {
    std::string code = R"(
        declare x: int;
        declare y: int;
        
        x = 10;
        y = 20;
        
        print(x);
        print(y);
        
        if (x == 10) {
            print(999);
        } else {
            print(111);
        }
        
        if (x == 5) {
            print(555);
        } else {
            print(222);
        }
        
        if (y == 20) {
            print(333);
        }
    )";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();

    std::cout << "=== TOKENS ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << static_cast<int>(token.type) << ": " 
                  << token.lexeme << std::endl;
    }
    std::cout << std::endl;

    Parser parser(tokens);
    auto program = parser.parseProgram();

    std::cout << "=== OUTPUT ===" << std::endl;
    Context ctx;
    program->execute(ctx);

    return 0;
}
