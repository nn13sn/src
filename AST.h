#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>

enum class Datatype{
    Int,
    Char,
    String,
    Double
};

struct AST{
    virtual ~AST() = default;
};

struct Statement : AST {};
struct Expression : AST {};

struct Program : AST {
    std::vector <std::unique_ptr<Statement>> statements;
};

struct Declaration : Statement {
     Datatype type;
     std::string name;
};

struct Output : Statement {
    std::unique_ptr <Expression> output;
};

struct Definition : Statement {
    std::string name;
    std::unique_ptr <Expression> value;
};

struct Number : Expression {
    int number;
};

struct Variable : Expression {
    std::string name;
};

struct Binary : Expression {
    char op;
    std::unique_ptr <Expression> right;
    std::unique_ptr <Expression> left;
};
