#include "VM.h"
#include "runtime_cast.h"
#include "runtime_operations.h"
#include "vm_error.h"

VM::VM(const uint32_t &size) { locals.resize(size); }

template <typename OperationFunc> void VM::BinaryOperation(OperationFunc func) {
  auto right = stack.Pop();
  auto left = stack.Pop();
  stack.Push(func(left, right));
}

template <typename OperationFunc> void VM::UnaryOperation(OperationFunc func) {
  auto value = stack.Pop();
  stack.Push(func(value));
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
        // std::cout << std::get<int64_t>(stack.Pop().getData());
        stack.Pop();
        break;
      case Action::Add:
        BinaryOperation(RuntimeOperations::Add);
        break;
      case Action::Sub:
        BinaryOperation(RuntimeOperations::Sub);
        break;
      case Action::Mul:
        BinaryOperation(RuntimeOperations::Mul);
        break;
      case Action::Div:
        BinaryOperation(RuntimeOperations::Div);
        break;
      case Action::Mod:
        BinaryOperation(RuntimeOperations::Mod);
        break;
      case Action::Equal:
        BinaryOperation(RuntimeOperations::Equal);
        break;
      case Action::NotEqual:
        BinaryOperation(RuntimeOperations::NotEqual);
        break;
      case Action::Less:
        BinaryOperation(RuntimeOperations::Less);
        break;
      case Action::Greater:
        BinaryOperation(RuntimeOperations::Greater);
        break;
      case Action::LessEq:
        BinaryOperation(RuntimeOperations::LessEq);
        break;
      case Action::GreaterEq:
        BinaryOperation(RuntimeOperations::GreaterEq);
        break;
      case Action::Neg:
        UnaryOperation(RuntimeOperations::Neg);
        break;
      case Action::Not:
        UnaryOperation(RuntimeOperations::Not);
        break;
      case Action::Cast:
        RuntimeCast::Cast(stack.Pop(),
                          static_cast<Datatype>(instruction.operand));
        break;
      case Action::Store_Local:
        locals[instruction.operand] = stack.Top();
        break;
      case Action::Load_Local:
        stack.Push(locals[instruction.operand].get());
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
