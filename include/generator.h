#pragma once
#include "AST.h"
#include "bytecode.h"
class Generator {
public:
  Generator(const Program &program);

private:
  void GenerateExpression(const Expression &expr);
  Bytecode code;
  uint32_t index = 0;

  void emit(const Action &action, const uint32_t &operand = 0);
};
