#include "generator.h"
#include "AST.h"
#include "utils.h"

size_t Generator::emit(Location location, const Action &action,
                       uint32_t operand) {
  auto index = code.code.size();
  code.code.emplace_back(Instruction(action, std::move(operand)));
  code.locations.push_back(std::move(location));
  return index;
}

void Generator::FinishJump(const size_t &instruction, const size_t &target) {
  code.code[instruction].operand = target;
}

void Generator::GenerateBody(const Program &program) {
  emit(program.location, Action::EnterScope);
  Generate(program);
  emit(program.location, Action::ExitScope);
}

void Generator::GenerateExpression(const Expression &expr) {
  switch (expr.ExpressionType) {
  case ExprType::exprValue:
    code.values.emplace_back(static_cast<const exprValue &>(expr).value);
    emit(expr.location, Action::Push_Value, code.values.size() - 1);
    return;
  case ExprType::Binary: {
    const Binary &binary = static_cast<const Binary &>(expr);
    GenerateExpression(*binary.left);
    GenerateExpression(*binary.right);
    emit(binary.location, utils::getOperatorAction(binary.op));
    return;
  }
  case ExprType::Assignment: {
    const Assignment &assignment = static_cast<const Assignment &>(expr);
    GenerateExpression(*assignment.right);
    emit(assignment.location, Action::Store_Local,
         indexes.slots
             [indexes.IDs[static_cast<const Variable &>(*assignment.left).id]]);
    return;
  }
  case ExprType::Variable:
    emit(expr.location, Action::Load_Local,
         indexes.slots[indexes.IDs[static_cast<const Variable &>(expr).id]]);
    return;
  case ExprType::Unary: {
    const Unary &unary = static_cast<const Unary &>(expr);
    auto op = utils::getOperatorAction(unary.op, false);
    if (op == Action::PreIncr || op == Action::PostIncr ||
        op == Action::PreDecr || op == Action::PostDecr) {
      auto index =
          indexes.slots
              [indexes.IDs[static_cast<const Variable &>(*unary.expr).id]];
      emit(expr.location, op, index);
      return;
    }
    GenerateExpression(*unary.expr);
    emit(expr.location, op);
    return;
  }
  case ExprType::Cast: {
    const auto &cast = static_cast<const Cast &>(expr);
    GenerateExpression(*cast.expr);
    emit(expr.location, Action::Cast, static_cast<uint32_t>(cast.castTo));
    return;
  }
  default:
    return;
  }
}

void Generator::GenerateOutput(const Output &stmt) {
  GenerateExpression(*stmt.output);
  emit(stmt.location, Action::Print);
}

void Generator::GenerateInput(const Input &stmt) {
  emit(stmt.location, Action::Read, static_cast<uint32_t>(stmt.InputType));
  emit(stmt.input->location, Action::Store_Local,
       indexes
           .slots[indexes.IDs[static_cast<const Variable &>(*stmt.input).id]]);
}

void Generator::GenerateIf(const IfStatement &stmt) {
  size_t escapejump;
  GenerateExpression(*stmt.expr);
  auto jump = emit(stmt.location, Action::JumpIfFalse);
  GenerateBody(*stmt.Instructions);
  if (stmt.elseStatement)
    escapejump = emit(stmt.elseStatement->location, Action::Jump);
  FinishJump(jump, code.code.size());
  if (stmt.elseStatement) {
    auto &elsestmt = static_cast<const IfStatement &>(*stmt.elseStatement);
    if (elsestmt.expr)
      GenerateIf(elsestmt);
    else
      GenerateBody(*elsestmt.Instructions);
    FinishJump(escapejump, code.code.size());
  }
}

const Bytecode &Generator::Generate(const Program &program) {
  for (const auto &stmt : program.statements) {
    switch (stmt->StatementType) {
    case StmtType::ExpressionStmt:
      GenerateExpression(*static_cast<const ExpressionStmt &>(*stmt).expr);
      emit(stmt->location, Action::Pop);
      break;
    case StmtType::Output:
      GenerateOutput(static_cast<const Output &>(*stmt));
      break;
    case StmtType::Input:
      GenerateInput(static_cast<const Input &>(*stmt));
      emit(stmt->location, Action::Pop);
      break;
    case StmtType::IfStatement:
      GenerateIf(static_cast<const IfStatement &>(*stmt));
      break;
    default:
      break;
    }
  }
  return code;
}
