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
    Parser parser(lexer);

    auto program = parser.parseProgram();

    Context ctx;
    program->execute(ctx);

    return 0;
}