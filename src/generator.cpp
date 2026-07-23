#include "generator.h"
#include "AST.h"
#include "utils.h"

void Generator::emit(const Location &location, const Action &action,
                     const uint32_t &operand) {
  code.code.emplace_back(Instruction(action, operand));
  code.locations.push_back(location);
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
    emit(binary.location,
         utils::getOperatorAction(static_cast<const Binary &>(expr).op));
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
  default:
    return;
  }
}

const Bytecode &Generator::Generate(const Program &program) {
  for (const auto &stmt : program.statements) {
    switch (stmt->StatementType) {
    case StmtType::ExpressionStmt:
      GenerateExpression(*static_cast<const ExpressionStmt &>(*stmt).expr);
      emit(stmt->location, Action::Pop);
      break;
    default:
      break;
    }
  }
  return code;
}
