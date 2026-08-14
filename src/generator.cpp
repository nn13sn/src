#include "generator.h"
#include "AST.h"
#include "utils.h"

void Generator::emit(Location location, const Action &action,
                     uint32_t operand) {
  code.code.emplace_back(Instruction(action, std::move(operand)));
  code.locations.push_back(std::move(location));
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
    emit(
        assignment.location, Action::Store_Local,
        indexes.slots.at(static_cast<const Variable &>(*assignment.left).name));
    return;
  }
  case ExprType::Variable:
    emit(expr.location, Action::Load_Local,
         indexes.slots.at(static_cast<const Variable &>(expr).name));
    return;
  case ExprType::Unary: {
    const Unary &unary = static_cast<const Unary &>(expr);
    auto op = utils::getOperatorAction(unary.op, false);
    if (op == Action::PreIncr || op == Action::PostIncr ||
        op == Action::PreDecr || op == Action::PostDecr) {
      auto index =
          indexes.slots.at(static_cast<const Variable &>(*unary.expr).name);
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
    return emit(expr.location, Action::Cast,
                static_cast<uint32_t>(cast.castTo));
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
       indexes.slots.at(static_cast<const Variable &>(*stmt.input).name));
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
    default:
      break;
    }
  }
  return code;
}
