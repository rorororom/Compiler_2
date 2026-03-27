#include "oop_nodes.h"
#include "visitor.h"

void FieldDecl::accept(Visitor* visitor)       { visitor->visit(this); }
void MethodDecl::accept(Visitor* visitor)      { visitor->visit(this); }
void ClassDecl::accept(Visitor* visitor)       { visitor->visit(this); }
void ReturnStmt::accept(Visitor* visitor)      { visitor->visit(this); }
void NewObjectExpr::accept(Visitor* visitor)   { visitor->visit(this); }
void NewArrayExpr::accept(Visitor* visitor)    { visitor->visit(this); }
void MethodCallExpr::accept(Visitor* visitor)  { visitor->visit(this); }
void FieldAccessExpr::accept(Visitor* visitor) { visitor->visit(this); }
void ThisExpr::accept(Visitor* visitor)        { visitor->visit(this); }
void ArrayAccessExpr::accept(Visitor* visitor) { visitor->visit(this); }
void ArrayLengthExpr::accept(Visitor* visitor) { visitor->visit(this); }
void VarDeclStmt::accept(Visitor* visitor)     { visitor->visit(this); }
