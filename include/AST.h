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

struct IfStatement : Statement {
  std::unique_ptr <Program> Instructions = std::make_unique <Program> ();
  std::unique_ptr <Expression> expr;
  std::unique_ptr <IfStatement> elseStatement = nullptr;
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

struct Logical : Expression {
  std::unique_ptr <Expression> right;
  std::unique_ptr <Expression> left;
  std::string op;
};
