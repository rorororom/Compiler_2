cmake_minimum_required(VERSION 3.16)

project(Compilers LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
    irreader
    native
    nativecodegen
)

set(AST_SOURCES
    ast/src/expressions.cpp
    ast/src/statements.cpp
    ast/src/program.cpp
    ast/src/oop_nodes.cpp
)

set(VISITOR_SOURCES
    visitor/src/visitor.cpp
    visitor/src/print_visitor.cpp
    visitor/src/interpreter_visitor.cpp
    visitor/src/scope_visitor.cpp
    visitor/src/llvm_visitor.cpp
)

set(SOURCES
    main.cpp
    frontend/src/lexer.cpp
    frontend/src/parser.cpp
    ${AST_SOURCES}
    ${VISITOR_SOURCES}
)

add_executable(compiler ${SOURCES})

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ast/include
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/visitor/include
    ${CMAKE_CURRENT_SOURCE_DIR}/semantic/include
)

target_link_libraries(compiler PRIVATE ${LLVM_LIBS})

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_scope_visitor.cpp
    frontend/src/lexer.cpp
    frontend/src/parser.cpp
    ${AST_SOURCES}
    ${VISITOR_SOURCES}
)

add_executable(compiler_tests ${TEST_SOURCES})

target_include_directories(compiler_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ast/include
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/visitor/include
    ${CMAKE_CURRENT_SOURCE_DIR}/semantic/include
)

target_link_libraries(compiler_tests PRIVATE
    gtest_main
    ${LLVM_LIBS}
)

add_test(NAME CompilerTests COMMAND compiler_tests)
