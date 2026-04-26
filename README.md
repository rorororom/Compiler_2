# Компилятор — Итерация 5: Нативная кодогенерация

## Что сделано

В этой итерации `clang` полностью исключён из пайплайна. Компилятор теперь самостоятельно собирает нативный исполняемый файл из LLVM IR-модуля, используя только LLVM C++ API.

### Ключевые изменения

**Target triple и DataLayout**

В конструкторе `LLVMVisitor` определяется хостовый target triple через `llvm::sys::getDefaultTargetTriple()`, он передаётся в модуль (`module_->setTargetTriple(triple)`), а затем через `llvm::TargetRegistry::lookupTarget()` создаётся `llvm::TargetMachine`. DataLayout модуля устанавливается из `targetMachine_->createDataLayout()`, что гарантирует корректное выравнивание и размеры типов для целевой платформы.

**Optimization passes (новый PassManager API)**

Метод `runOptimizationPasses(int optLevel)` использует `llvm::PassBuilder` (новый PassManager, LLVM 14+):
- `O0` — `buildO0DefaultPipeline` (без оптимизаций)
- `O1/O2/O3` — `buildPerModuleDefaultPipeline` с соответствующим `OptimizationLevel`

**Эмиссия нативного объектного файла**

Метод `compileToObject(objFile, optLevel)`:
1. Запускает оптимизационные проходы
2. Открывает `raw_fd_ostream` для `.o`-файла
3. Вызывает `targetMachine_->addPassesToEmitFile(..., CGFT_ObjectFile)` через `legacy::PassManager`
4. Запускает `codegenPM.run(*module_)`

**Линковка в исполняемый файл**

Метод `compileToExecutable(outFile, optLevel)`:
1. Вызывает `compileToObject()` во временный `.o`-файл
2. Запускает системный линкер (`cc`) через `std::system()`
3. Удаляет временный `.o`-файл

## Структура пайплайна

```
исходный файл
  → Lexer → Parser → AST
  → ScopeVisitor (семантика)
  → LLVMVisitor (IR-генерация)
      ├─ setTargetTriple / setDataLayout
      ├─ PassBuilder (O0–O3)
      ├─ addPassesToEmitFile → output.o
      └─ cc output.o -o program
  → нативный исполняемый файл (без clang!)
```

## Флаги

```
compiler <файл> [опции]

  -o <файл>            выходной .ll файл (по умолчанию output.ll)
  --emit-obj <файл>    эмитировать нативный объектный файл (.o)
  --compile <файл>     скомпилировать в нативный исполняемый файл
  --opt-level <N>      уровень оптимизации: 0 (нет), 1, 2, 3 (по умолчанию: 0)
  --print-ir           вывести IR в stdout
  --print-ast          записать AST в ast_output.txt
  --dump-scope         записать дерево скоупов в scope_tree_output.txt
  --help               справка
```

## Сборка и запуск

Нужен Docker.

```bash
cd Compiler_2
docker build -t compiler5 .
```

Первый запуск занимает несколько минут — скачивается LLVM 17.

### Запуск примеров (нативная компиляция без clang)

```bash
# Простой пример — компиляция напрямую в исполняемый файл
docker run --rm compiler5 /bin/bash -c \
  "./build/compiler example/example.txt --compile prog && ./prog"

# С оптимизацией O2
docker run --rm compiler5 /bin/bash -c \
  "./build/compiler example/example_1.txt --compile prog --opt-level 2 && ./prog"

# OOP-пример
docker run --rm compiler5 /bin/bash -c \
  "./build/compiler example/example_oop.txt --compile prog && ./prog"


### Интерактивная оболочка

```bash
docker run --rm -it compiler5
```

Внутри контейнера:

```bash
# Скомпилировать в нативный бинарник (без clang)
./build/compiler example/example.txt --compile program --opt-level 2
./program

# Посмотреть IR и объектный файл
./build/compiler example/example.txt --print-ir --emit-obj out.o

# Только IR
./build/compiler example/example.txt -o output.ll --print-ir
```

### Через скрипт

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

# Нативная компиляция без clang
./build/compiler example/example.txt --compile program --opt-level 2
./program
```

## Примеры

`example/example.txt` — `10 + 20 != 31`, выводит `111`.

`example/example_1.txt` — `6 * 7 = 42`, выводит `1` и `100`.

`example/example_oop.txt` — классы с полями и методами, выводит `1`.

`example/example_shadowing.txt` — shadowing переменных в блоках.

## Уровни оптимизации

| Флаг | Уровень | Описание |
|------|---------|----------|
| `--opt-level 0` | O0 | Без оптимизаций (по умолчанию) |
| `--opt-level 1` | O1 | Базовые оптимизации |
| `--opt-level 2` | O2 | Стандартные оптимизации (рекомендуется) |
| `--opt-level 3` | O3 | Агрессивные оптимизации |
