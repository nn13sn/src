#pragma once
#include "AST.h"
#include "bytecode.h"
#include "slot_table.h"
class Generator {
public:
  const Bytecode &Generate(const Program &program);
  const Slot_Table &indexes;
  Generator(const Slot_Table &table) : indexes(table) {};

private:
  void GenerateExpression(const Expression &expr);
  void GenerateOutput(const Output &stmt);
  void GenerateInput(const Input &stmt);
  Bytecode code;
  uint32_t index = 0;

  void emit(Location location, const Action &action, uint32_t operand = 0);
};
