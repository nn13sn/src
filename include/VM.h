#pragma once
#include "bytecode.h"
#include "stack.h"

class VM {
public:
  void evaluate(const Bytecode &code);

private:
  Stack stack;
  size_t instruction_number = 0;
  void Add();
  void Sub();
  void Mul();
  void Div();
};
