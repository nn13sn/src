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
    GenerateExpression(*unary.expr);
    emit(expr.location, utils::getOperatorAction(unary.op, false));
    return;
  }
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
