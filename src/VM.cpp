#include "VM.h"

void VM::Add() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) +
                                      std::get<int64_t>(right.getData())));
}

void VM::Sub() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) -
                                      std::get<int64_t>(right.getData())));
}

void VM::Mul() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) *
                                      std::get<int64_t>(right.getData())));
}

void VM::Div() {
  auto right = stack.Pop();
  auto left = stack.Pop();

  stack.Push(
      RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) /
                                      std::get<int64_t>(right.getData())));
}

void VM::evaluate(const Bytecode &code) {
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
      break;
    }
  }
}
