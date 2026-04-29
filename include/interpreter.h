#pragma once
#include "AST.h"
#include "runtime_value.h"
#include "utils.h"
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

struct Environment;

class interpreter_error : public std::runtime_error {
public:
  Location location;
  interpreter_error(const std::string &msg, const Location &loc);
};

struct Function {
  const FunctionStatement *declaration;
};

struct ReturnException {
  RuntimeValue value;
};

struct Environment {
  std::unordered_map<std::string, RuntimeValue> values = {};
  inline static std::unordered_map<std::string, RuntimeValue> globals = {};
  std::shared_ptr<Environment> parent = nullptr;
  RuntimeValue get(const std::string &name);
  RuntimeValue *getPointer(const std::string &name);
  void set(const std::string &name, const RuntimeValue &value);
  bool newGlobal(const std::string &name, const RuntimeValue &value);
};

class Interpreter {
public:
  void execute(const Program &program);

private:
  bool insidefunction = false;
  int32_t currentmods = 0;
  std::shared_ptr<Environment> environment = std::make_shared<Environment>();
  void matchStatement(const Statement &stmt);
  void input(const Input &stmt);
  void output(const Output &stmt);
  void expression(const ExpressionStmt &stmt);
  void whileloop(const While &stmt);
  void forloop(const For &stmt);
  bool getCond(const int64_t &Initial, const int64_t Final,
               const signed char &direction, const Operator &op);
  void forbody(RuntimeValue *&Initial, const signed char &direction,
               const For &stmt);
  void ifStatement(const IfStatement &stmt);
  void function(const FunctionStatement &stmt);
  void returnStatement(const ReturnStatement &stmt);
  void block(const BlockStatement &stmt);
  double toDouble(const RuntimeValue &value, const Location &loc);
  int64_t toInt(const RuntimeValue &value, const Location &loc);
  std::string toString(const RuntimeValue &value, const Location &loc);
  unsigned char toChar(const RuntimeValue &value, const Location &loc);
  RuntimeValue convertString(const Cast &expr);
  bool isTrue(const RuntimeValue &value, const Location &loc);
  RuntimeValue eval(const Expression &expr);
  RuntimeValue evalFunctionCall(const FunctionCall &expr);
  RuntimeValue evalNegative(const Unary &expr);
  RuntimeValue evalPreIncr(const Unary &expr);
  RuntimeValue evalPreDecr(const Unary &expr);
  RuntimeValue evalPostIncr(const Unary &expr);
  RuntimeValue evalPostDecr(const Unary &expr);
  RuntimeValue evalDef(const Binary &expr);
  RuntimeValue evalAdd(const RuntimeValue &left, const RuntimeValue &right,
                       const Location &loc);
  RuntimeValue evalSub(const RuntimeValue &left, const RuntimeValue &right,
                       const Location &loc);
  RuntimeValue evalMul(const RuntimeValue &left, const RuntimeValue &right,
                       const Location &loc);
  RuntimeValue evalDiv(const RuntimeValue &left, const RuntimeValue &right,
                       const Location &loc);
  RuntimeValue evalMod(const RuntimeValue &left, const RuntimeValue &right,
                       const Location &loc);
  RuntimeValue evalGr(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  RuntimeValue evalLs(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  RuntimeValue evalEq(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  RuntimeValue evalNq(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  RuntimeValue evalGe(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  RuntimeValue evalLe(const RuntimeValue &left, const RuntimeValue &right,
                      const Location &loc);
  void addScope();
  void popScope();
  RuntimeValue validCheck(const RuntimeValue &value, const Location &loc,
                          const std::string &name);
  RuntimeValue *validCheck(RuntimeValue *ptr, const Location &loc,
                           const std::string &name);
};
