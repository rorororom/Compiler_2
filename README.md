# Компилятор

### Сборка и запуск

```bash
mkdir build && cd build
cmake ..
make
./compiler ../example.txt
./compiler_tests
```


## Итерация 1. Лексер и Парсер

Реализация лексического анализатора и синтаксического анализатора для простого языка программирования.

### Требования задания
- **Определение переменных:** `declare x: int;`
- **Присваивание значений:** `x = 0;`
- **Условные выражения:** `if (x == 0) { then_stmt } else { else_stmt }`
- **Функция print:** `print(int)`
- **Синтаксическое дерево:** базовый класс для каждого нетерминала, производные классы для правил

## Итерация 2. Визиторы

Реализация паттерна Visitor для обхода AST.

### Что добавлено
- **Базовый класс Visitor** (`visitor/include/visitor.h`) - интерфейс для всех визиторов
- **PrintVisitor** (`visitor/src/print_visitor.cpp`) - печатает AST в файл `ast_output.txt`
- **InterpreterVisitor** (`visitor/src/interpreter_visitor.cpp`) - интерпретирует и выполняет программу

### Как работает
```cpp
PrintVisitor printVisitor("ast_output.txt");
InterpreterVisitor interpreter;

program->accept(&printVisitor);  // Печать структуры
program->accept(&interpreter);   // Выполнение программы
```

### Примеры

#### Простой пример
```
declare x: int;
x = 42;
print(x);
```

#### С условиями
```
declare x: int;
x = 10;
if (x == 10) {
    print(999);
} else {
    print(111);
}
```

#### Грамматика языка
```txt
program        → statement* EOF

statement      → declStatement
               | assignStatement  
               | printStatement
               | ifStatement

declStatement  → "declare" IDENTIFIER ":" "int" ";"

assignStatement → IDENTIFIER "=" expression ";"

printStatement → "print" "(" expression ")" ";"

ifStatement    → "if" "(" expression ")" "{" statement* "}" 
                 ( "else" "{" statement* "}" )?

expression     → equality

equality       → addition ( "==" addition )*

addition       → multiplication ( ("+" | "-") multiplication )*

multiplication → primary ( ("*" | "/") primary )*

primary        → NUMBER
               | IDENTIFIER
               | "(" expression ")"
```

#### Лексические правила
```txt
IDENTIFIER     → [a-zA-Z][a-zA-Z0-9]*
NUMBER         → [0-9]+
KEYWORD        → "declare" | "int" | "if" | "else" | "print"
OPERATOR       → "+" | "-" | "*" | "/" | "==" | "="
DELIMITER      → ":" | ";" | "(" | ")" | "{" | "}"
WHITESPACE     → [ \t\n\r]+ (игнорируется)
```
