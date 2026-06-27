#pragma once
#include "datatype.h"
#include "funcparameter.h"
#include "literal.h"
#include "location.h"
#include "operators.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#define MAX_ERROR_NUMBER 20

enum class ExprType;

enum Modifiers { MOD_NONE = 0, MOD_GLOBAL = 1, MOD_DYNAMIC = 2, MOD_CONST = 4 };

enum class StmtType {
  Input,
  Output,
  ExpressionStmt,
  IfStatement,
  While,
  For,
  FunctionStatement,
  ReturnStatement,
  BlockStatement,
  Amount
};

struct AST {
  Location location;
  virtual ~AST() = default;
};

struct Statement : AST {
  StmtType StatementType;
  int32_t mods = MOD_NONE;
};

struct Expression : AST {
  ExprType ExpressionType;
};

struct Program : AST {
  std::vector<std::unique_ptr<Statement>> statements;
};

struct Input : Statement {
  std::unique_ptr<Expression> input;
};

struct Output : Statement {
  std::unique_ptr<Expression> output;
};

struct ExpressionStmt : Statement {
  std::unique_ptr<Expression> expr;
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
  std::string iterator;
  Operator op;
  std::unique_ptr<Expression> step = nullptr;
  std::unique_ptr<Expression> Initialvalue;
  std::unique_ptr<Expression> Finalvalue;
  std::unique_ptr<Program> Instructions;
};

struct FunctionStatement : Statement {
  std::string name;
  std::vector<Parameter> params = {};
  std::unique_ptr<Program> Instructions;
};

struct ReturnStatement : Statement {
  std::unique_ptr<Expression> expr;
};

struct BlockStatement : Statement {
  std::unique_ptr<Program> instructions;
};

enum class ExprType {
  exprValue,
  Variable,
  FunctionCall,
  Binary,
  Unary,
  Cast,
  Amount
};

struct exprValue : Expression {
  Literal value;
};

struct Variable : Expression {
  std::string name;
};

struct FunctionCall : Expression {
  std::string name;
  std::vector<std::unique_ptr<Expression>> parameters = {};
};

struct Binary : Expression {
  Operator op;
  std::unique_ptr<Expression> right;
  std::unique_ptr<Expression> left;
};

struct Unary : Expression {
  Operator op;
  std::unique_ptr<Expression> expr;
};

struct Cast : Expression {
  Datatype castTo;
  std::unique_ptr<Expression> expr;
};
