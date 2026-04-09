# Компилятор 
## Что сделано

Написан `LLVMVisitor` — визитор, который обходит AST и генерирует LLVM IR через C++ API (IRBuilder). Каждый узел дерева преобразуется в соответствующую инструкцию:

- переменные — `alloca` + `load`/`store`
- арифметика — `add`, `sub`, `mul`, `sdiv`
- сравнение — `icmp eq` + `zext`
- `if/else` — условный переход с блоками `then`/`else`/`merge`
- `print` — вызов `printf("%d\n", ...)`
- классы — именованные LLVM-структуры, методы — функции с неявным `self`
- `new ClassName()` — `malloc` + `memset` + bitcast
- `new int[n]` — `malloc` + bitcast в `i32*`

Парсинг аргументов командной строки переписан на `llvm::cl`.

Сборка упакована в Docker — Ubuntu 22.04 + LLVM 17 + clang-17. Сгенерированный `.ll` файл компилируется `clang`-ом в нативный бинарник.

## Структура пайплайна

```
исходный файл -> Lexer -> Parser -> AST -> ScopeVisitor -> LLVMVisitor -> output.ll -> clang -> бинарник
```

## Флаги

```
compiler <файл> [опции]

  -o <файл>      выходной .ll файл (по умолчанию output.ll)
  --print-ir     вывести IR в stdout
  --print-ast    записать AST в ast_output.txt
  --dump-scope   записать дерево скоупов в scope_tree_output.txt
  --help         справка
```

## Сборка и запуск

Нужен Docker.

```bash
cd Compiler_2
docker build -t compiler2 .
```

Первый запуск занимает несколько минут — скачивается LLVM 17.

Запуск примеров:

```bash
docker run --rm compiler2 /bin/bash -c \
  "./build/compiler example/example.txt -o out.ll && clang out.ll -o prog && ./prog"

docker run --rm compiler2 /bin/bash -c \
  "./build/compiler example/example_1.txt -o out.ll && clang out.ll -o prog && ./prog"

docker run --rm compiler2 /bin/bash -c \
  "./build/compiler example/example_oop.txt -o out.ll && clang out.ll -o prog && ./prog"
```

Интерактивная оболочка:

```bash
docker run --rm -it compiler2
```

Внутри контейнера:

```bash
./build/compiler example/example.txt -o output.ll --print-ir
clang output.ll -o program
./program
```

Через скрипт:

```bash
chmod +x build.sh
./build.sh build
./build.sh run example/example.txt
./build.sh shell
./build.sh test
```

## Локальная сборка

Если LLVM 17 уже установлен:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_DIR=$(llvm-config-17 --cmakedir)
cmake --build build -j$(nproc)
./build/compiler example/example.txt -o output.ll
clang output.ll -o program && ./program
```

## Примеры

`example/example.txt` — `10 + 20 != 31`, выводит `111`.

`example/example_1.txt` — `6 * 7 = 42`, выводит `1` и `100`.

`example/example_oop.txt` — классы с полями и методами, выводит `1`. Классы преобразуются в LLVM-структуры, методы — в функции вида `Point__getX(%Point* %self)`.
