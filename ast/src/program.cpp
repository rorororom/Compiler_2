#include "program.h"
#include "visitor.h"

void Program::accept(Visitor* visitor) {
    visitor->visit(this);
}