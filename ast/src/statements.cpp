#include "statements.h"
#include "visitor.h"
#include "context.h"

void DeclareStmt::execute(Context& ctx) {
    ctx.declare(name);
}

void DeclareStmt::accept(Visitor* visitor) {
    visitor->visit(this);
}

void AssignStmt::execute(Context& ctx) {
    ctx.assign(name, expr->evaluate(ctx));
}

void AssignStmt::accept(Visitor* visitor) {
    visitor->visit(this);
}

void PrintStmt::execute(Context& ctx) {
    ctx.print(expr->evaluate(ctx));
}

void PrintStmt::accept(Visitor* visitor) {
    visitor->visit(this);
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

void IfStmt::accept(Visitor* visitor) {
    visitor->visit(this);
}