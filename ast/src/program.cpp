#include "program.h"
#include "oop_nodes.h"
#include "visitor.h"

// Destructor defined here so that ClassDecl is a complete type
// (required by std::unique_ptr<ClassDecl> in the classes vector).
Program::~Program() = default;

void Program::accept(Visitor* visitor) {
    visitor->visit(this);
}