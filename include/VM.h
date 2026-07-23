#pragma once
#include "bytecode.h"
#include "stack.h"
#define VM_ERROR -1
#define VM_OK 1
class VM {
public:
  VM(const uint32_t &size);
  signed char evaluate(const Bytecode &code);

private:
  Stack stack;
  std::vector<RuntimeValue> locals = {};
  size_t instruction_number = 0;
  void Add();
  void Sub();
  void Mul();
  void Div();
};
