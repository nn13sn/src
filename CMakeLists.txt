cmake_minimum_required(VERSION 3.16)

project(DoubleC LANGUAGES CXX)

add_executable(interpreterA
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)

target_compile_features(interpreterA PRIVATE cxx_std_20)

target_compile_options(interpreterA PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

target_include_directories(interpreterA PRIVATE include)
