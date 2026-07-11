#include "VM.h"
#include "vm_error.h"

void VM::Add() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"+\" cannot be used with such data type");

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) +
                                      std::get<int64_t>(right.getData())));
}

void VM::Sub() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"-\" cannot be used with such data type");

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) -
                                      std::get<int64_t>(right.getData())));
}

void VM::Mul() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"*\" cannot be used with such data type");

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) *
                                      std::get<int64_t>(right.getData())));
}

void VM::Div() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"/\" cannot be used with such data type");

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) /
                                      std::get<int64_t>(right.getData())));
}

signed char VM::evaluate(const Bytecode &code) {
  try {
    while (instruction_number < code.code.size()) {
      const Instruction &instruction = code.code[instruction_number++];
      switch (instruction.action) {
      case Action::Push_Value:
        stack.Push(code.values[instruction.operand]);
        break;
      case Action::Pop:
        stack.Pop();
        break;
      case Action::Add:
        Add();
        break;
      case Action::Sub:
        Sub();
        break;
      case Action::Mul:
        Mul();
        break;
      case Action::Div:
        Div();
        break;
      default:
        throw VM_error("Unknown instruction");
      }
    }
  } catch (const VM_error &err) {
    std::cerr << "\nRuntime error: " << err.what()
              << " at line: " +
                     std::to_string(
                         code.locations[instruction_number - 1].line);
    std::cerr << "; column: " +
                     std::to_string(
                         code.locations[instruction_number - 1].column)
              << '\n';
    return VM_ERROR;
  }
  return VM_OK;
}
