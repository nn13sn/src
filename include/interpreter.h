#pragma once
#include "AST.h"
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

class interpreter_error : public std::runtime_error {
public:
  Location location;
  interpreter_error(const std::string &msg, const Location &loc);
};

struct Environment {
  std::unordered_map<std::string, Value> values = {};
  std::shared_ptr<Environment> parent = nullptr;
  Value get(const std::string &name);
  Value *getPointer(const std::string &name);
  void set(const std::string &name, const Value &value);
};

class Interpreter {
public:
  void execute(const Program &program);

private:
  std::shared_ptr<Environment> environment = std::make_shared<Environment>();
  void matchStatement(const Statement &stmt);
  void input(const Input &stmt);
  void output(const Output &stmt);
  void expression(const ExpressionStmt &stmt);
  void whileloop(const While &stmt);
  void forloop(const For &stmt);
  void forbody(Value *&Initial, const short &direction, const For &stmt);
  void ifStatement(const IfStatement &stmt);
  double toDouble(const Value &value);
  int64_t toInt(const Value &value);
  std::string toString(const Value &value);
  char toChar(const Value &value);
  Value convertString(const Cast &expr);
  bool isNumeric(const Value &value);
  bool isTrue(const Value &value);
  Value eval(const Expression &expr);
  Value evalNegative(const Unary &expr);
  Value evalPreIncr(const Unary &expr);
  Value evalPreDecr(const Unary &expr);
  Value evalPostIncr(const Unary &expr);
  Value evalPostDecr(const Unary &expr);
  Value evalDef(const Binary &expr);
  Value evalAdd(const Value &left, const Value &right);
  Value evalSub(const Value &left, const Value &right);
  Value evalMul(const Value &left, const Value &right);
  Value evalDiv(const Value &left, const Value &right);
  Value evalMod(const Value &left, const Value &right);
  Value evalGr(const Value &left, const Value &right);
  Value evalLs(const Value &left, const Value &right);
  Value evalEq(const Value &left, const Value &right);
  Value evalNq(const Value &left, const Value &right);
  Value evalGe(const Value &left, const Value &right);
  Value evalLe(const Value &left, const Value &right);
  void addScope();
  void popScope();
  Value validCheck(const Value &value, const Location &loc,
                   const std::string &name);
  Value *validCheck(Value *ptr, const Location &loc, const std::string &name);
};
