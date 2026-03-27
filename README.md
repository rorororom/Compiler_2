# Compiler Project

A mini-compiler for a simple OOP language, built incrementally across checkpoints.

---

## Project Structure

```
Compiler_2/
├── frontend/          # Lexer + Parser (source → tokens → AST)
│   ├── include/
│   │   ├── token.h        — TokenType enum, Token struct, keyword maps
│   │   ├── lexer.h        — Lexer declaration
│   │   └── parser.h       — Parser declaration
│   └── src/
│       ├── lexer.cpp      — Tokeniser (skips // comments)
│       └── parser.cpp     — Recursive-descent parser
│
├── ast/               # Abstract Syntax Tree node definitions
│   ├── include/
│   │   ├── node.h         — Base Node (execute + accept)
│   │   ├── expression.h   — Base Expression
│   │   ├── expressions.h  — NumberExpr, VariableExpr, BinaryExpr, EqualExpr
│   │   ├── statements.h   — DeclareStmt, AssignStmt, PrintStmt, IfStmt
│   │   ├── program.h      — Program (root node)
│   │   └── oop_nodes.h    — ClassDecl, FieldDecl, MethodDecl, ReturnStmt,
│   │                        VarDeclStmt, NewObjectExpr, NewArrayExpr,
│   │                        MethodCallExpr, FieldAccessExpr, ThisExpr,
│   │                        ArrayAccessExpr, ArrayLengthExpr
│   └── src/
│       ├── expressions.cpp
│       ├── statements.cpp
│       ├── program.cpp
│       └── oop_nodes.cpp  — accept() dispatch for OOP nodes
│
├── semantic/          # Middle-end: semantic analysis
│   └── include/
│       ├── symbol_table.h — TypeInfo, VariableSymbol, MethodSymbol,
│       │                    ClassSymbol (fields + methods + constructor★),
│       │                    SymbolTable (global class registry)
│       └── scope_tree.h   — ScopeKind, Scope (tree node with locals map),
│                            ScopeTree (stack-based push/pop API)
│
├── visitor/           # Visitor pattern implementations
│   ├── include/
│   │   ├── visitor.h              — Abstract Visitor base (all visit() overloads)
│   │   ├── print_visitor.h        — AST pretty-printer to file
│   │   ├── interpreter_visitor.h  — Tree-walking interpreter
│   │   └── scope_visitor.h        — Scope tree builder (two-pass)
│   └── src/
│       ├── print_visitor.cpp
│       ├── interpreter_visitor.cpp
│       └── scope_visitor.cpp      — Pass 1: collectClasses → SymbolTable
│                                    Pass 2: accept → ScopeTree
│
├── backend/           # Back-end (IR generation / code emission — future)
│   └── include/
│       └── context.h  — Runtime variable map (used by interpreter visitor)
│
├── tests/
│   ├── test_lexer.cpp         — 34 lexer tests
│   ├── test_parser.cpp        — 45 parser tests
│   └── test_scope_visitor.cpp — 56 semantic analysis tests
│
├── example/
│   ├── example.txt            — Basic arithmetic + if/else
│   ├── example_1.txt          — Variables + arithmetic
│   ├── example_oop.txt        — Two classes with fields and methods
│   └── example_shadowing.txt  — Variable shadowing (warning demo)
│
├── main.cpp           — Entry point: lex → parse → print AST → scope analysis
└── CMakeLists.txt
```

---

## Architecture

```
Source code
    │
    ▼
┌─────────┐
│  Lexer  │  frontend/src/lexer.cpp
└────┬────┘
     │ tokens
     ▼
┌─────────┐
│ Parser  │  frontend/src/parser.cpp
└────┬────┘
     │ AST (Program*)
     ▼
┌──────────────────────────────────────────────────────┐
│                   Visitor passes                     │
│                                                      │
│  PrintVisitor      — pretty-print AST to file        │
│  ScopeVisitor      — two-pass semantic analysis:     │
│    Pass 1: collectClasses → SymbolTable              │
│    Pass 2: accept         → ScopeTree                │
│  InterpreterVisitor — tree-walking execution         │
└──────────────────────────────────────────────────────┘
     │
     ▼
  backend/  (IR generation — next checkpoint)
```

---

## Semantic Analysis (Checkpoint 2)

### Symbol Table (`semantic/include/symbol_table.h`)

Leaf-level data referenced by scope nodes:

| Type | Description |
|------|-------------|
| `TypeInfo` | Type descriptor: `INT`, `CLASS_INST`, `ARRAY_INT`, `ARRAY_CLASS` |
| `VariableSymbol` | name + type + storage kind (`LOCAL`/`FIELD`/`PARAM`) + slot index |
| `MethodSymbol` | name, return type, ordered params, owner class, `isConstructor` flag |
| `ClassSymbol` | name, private fields (indexed), public methods map |
| `SymbolTable` | Global `unordered_map<string, ClassSymbol>` |

**★ Constructor handling:** `ClassSymbol::ensureConstructor()` synthesises a
`MethodSymbol` with `isConstructor = true` stored under the class name.
`visit(NewObjectExpr*)` resolves it and uses `cs->fieldCount()` to determine
how many words of memory the IR must allocate.

### Scope Tree (`semantic/include/scope_tree.h`)

```
[global]
  var x : int (local)
  [class 'Point']
    var x : int (field)
    var y : int (field)
    [method 'Point::getX']
    [method 'Point::sum']
      var result : int (local)
  [block 'if-then']
    var x : int (local)   ← shadows outer x (warning)
  [block 'if-else']
```

- `Scope::declare()` — throws `SemanticError` on double-declaration; warns on shadowing
- `Scope::lookup()` — walks parent chain (innermost binding wins)
- `ScopeTree::pushScope()` / `popScope()` — classic stack algorithm

### Semantic errors detected

| Error | Message |
|-------|---------|
| Variable declared twice in same scope | `SemanticError: variable 'x' is already declared in this scope` |
| Variable used before declaration | `SemanticError: variable 'z' used before declaration` |
| Variable shadows outer declaration | `Warning: variable 'x' shadows an outer declaration` |
| `this` outside a method | `SemanticError: 'this' used outside of a class method` |
| Unknown class in `new` | `SemanticError: unknown class 'Foo'` |
| Wrong constructor arg count | `SemanticError: constructor 'Foo' expects N argument(s), got M` |

---

## Building

```bash
mkdir -p Compiler_2/build && cd Compiler_2/build
cmake ..
cmake --build . --target compiler
cmake --build . --target compiler_tests
./compiler_tests
```

## Running

```bash
./compiler ../example/example_oop.txt
./compiler ../example/example_shadowing.txt
```

Output files: `ast_output.txt`, `scope_tree_output.txt`

---

## Language Syntax

```
program     ::= statement*
statement   ::= classDecl
              | 'declare' ID ':' 'int' ';'
              | type ID ['=' expr] ';'
              | ID '=' expr ';'
              | 'if' '(' expr ')' '{' statement* '}' ['else' '{' statement* '}']
              | 'print' '(' expr ')' ';'
              | 'return' [expr] ';'

classDecl   ::= 'class' ID '{' member* '}'
member      ::= type ID ';'                          // field
              | type ID '(' params ')' '{' body '}'  // method

type        ::= ('int' | 'void' | ID) ['[]']
params      ::= (type ID (',' type ID)*)?

expr        ::= equality
equality    ::= addition ('==' addition)*
addition    ::= mult (('+' | '-') mult)*
mult        ::= postfix (('*' | '/') postfix)*
postfix     ::= primary ('.' ID ['(' args ')'] | '[' expr ']')*
primary     ::= NUMBER | 'this' | ID
              | 'new' type '(' args ')'
              | 'new' type '[' expr ']'
              | '(' expr ')'
```
