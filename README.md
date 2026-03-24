# Компилятор

## Итерация 0. Лексер и Парсер

Реализация лексического анализатора и синтаксического анализатора для простого языка программирования.

### Требования задания
- **Определение переменных:** `declare x: int;`
- **Присваивание значений:** `x = 0;`
- **Условные выражения:** `if (x == 0) { then_stmt } else { else_stmt }`
- **Функция print:** `print(int)`
- **Синтаксическое дерево:** базовый класс для каждого нетерминала, производные классы для правил


### Сборка и запуск

```bash
mkdir build && cd build
cmake ..
make
./compiler
./compiler_tests
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

####
Грамматика языка
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