#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class Datatype : uint8_t {
  Int,
  Char,
  String,
  Double,
  Bool,
  Array,
  Invalid
};

enum class Operator : uint8_t {
  Add,
  Sub,
  Mul,
  Div,
  Mod,

  Not,
  Less,
  Greater,
  LessEq,
  GreaterEq,
  Equal,
  NotEqual,
  AND,
  OR,

  Def,
  Arrow,
  ArrowEq,
  Invalid
};

enum class Separator : uint8_t {
  LeftParenthesis,
  RightParenthesis,
  Colon,
  Semicolon,
  LeftSquareBracket,
  RightSquareBracket,
  LeftCurlyBracket,
  RightCurlyBracket,
  Invalid
};

struct Value {
  Datatype type;
  std::variant<int64_t, char, std::string, double, bool, std::vector<Value>>
      data;
};

struct Location {
  size_t column = 0;
  size_t line;
};

struct AST {
  Location location;
  virtual ~AST() = default;
};

struct Statement : AST {};
struct Expression : AST {};

struct Program : AST {
  std::vector<std::unique_ptr<Statement>> statements;
};

struct Declaration : Statement {
  Datatype type;
  std::string name;
};

struct Input : Statement {
  std::unique_ptr<Expression> input;
};

struct Output : Statement {
  std::unique_ptr<Expression> output;
};

struct Definition : Statement {
  std::string name;
  std::unique_ptr<Expression> value;
};

struct IfStatement : Statement {
  std::unique_ptr<Program> Instructions = std::make_unique<Program>();
  std::unique_ptr<Expression> expr;
  std::unique_ptr<IfStatement> elseStatement = nullptr;
};

struct While : Statement {
  std::unique_ptr<Program> Instructions = std::make_unique<Program>();
  std::unique_ptr<Expression> expr;
};

struct For : Statement {
  Operator op;
  std::unique_ptr<Definition> step = nullptr;
  std::unique_ptr<Definition> Initialvalue = std::make_unique<Definition>();
  std::unique_ptr<Expression> Finalvalue;
  std::unique_ptr<Program> Instructions;
};

struct exprValue : Expression {
  Value value;
};

struct Variable : Expression {
  std::string name;
};

struct Binary : Expression {
  Operator op;
  std::unique_ptr<Expression> right;
  std::unique_ptr<Expression> left;
};

struct Cast : Expression {
  Datatype castTo;
  std::unique_ptr<Expression> expr;
};
