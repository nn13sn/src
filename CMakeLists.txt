cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(DoubleC LANGUAGES CXX)

add_executable(DoubleC
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/analyzer.cpp
    src/runtime_value.cpp
    src/runtime_variable.cpp
    src/environment.cpp
    src/utils.cpp
    src/generator.cpp
    src/VM.cpp
    src/runtime_operations.cpp
    src/runtime_cast.cpp
    src/runtime_streams.cpp
)

target_compile_features(DoubleC PRIVATE cxx_std_20)

target_compile_options(DoubleC PRIVATE
    -Wall
    -Wextra
    -g
    -O0
    -fsanitize=address
    -fno-omit-frame-pointer
)

target_link_options(DoubleC PRIVATE
    -fsanitize=address
)

target_include_directories(DoubleC PRIVATE include)
