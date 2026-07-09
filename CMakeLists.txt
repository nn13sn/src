cmake_minimum_required(VERSION 3.16)

project(DoubleC LANGUAGES CXX)

add_executable(DoubleC
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/analyzer.cpp
    src/runtime_value.cpp
    src/runtime_variable.cpp
    src/environment.cpp
    src/utils.cpp
    src/generator.cpp
    src/VM.cpp
)

target_compile_features(DoubleC PRIVATE cxx_std_20)

target_compile_options(DoubleC PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

target_include_directories(DoubleC PRIVATE include)
