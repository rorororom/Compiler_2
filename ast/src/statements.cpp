#include "statements.h"
#include "context.h"

void DeclareStmt::execute(Context& ctx) {
    ctx.declare(name);
}

void AssignStmt::execute(Context& ctx) {
    ctx.assign(name, expr->evaluate(ctx));
}

void PrintStmt::execute(Context& ctx) {
    ctx.print(expr->evaluate(ctx));
}

void IfStmt::execute(Context& ctx) {
    if (cond->evaluate(ctx)) {
        for (auto& stmt : thenBranch)
            stmt->execute(ctx);
    } else {
        for (auto& stmt : elseBranch)
            stmt->execute(ctx);
    }
}
