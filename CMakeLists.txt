cmake_minimum_required(VERSION 3.16)

project(Compilers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    frontend/lexer.cpp
    frontend/parser.cpp
)

add_executable(compiler ${SOURCES})

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include/ast
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/include
)

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
    frontend/lexer.cpp
    frontend/parser.cpp
)

add_executable(compiler_tests ${TEST_SOURCES})

target_include_directories(compiler_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include/ast
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/include
)

target_link_libraries(compiler_tests
    gtest_main
)

add_test(NAME CompilerTests COMMAND compiler_tests)