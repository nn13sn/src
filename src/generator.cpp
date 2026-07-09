#include "generator.h"
#include "AST.h"
#include "utils.h"

void Generator::emit(const Action &action, const uint32_t &operand) {
  code.code.emplace_back(Instruction(action, operand));
}

void Generator::GenerateExpression(const Expression &expr) {
  switch (expr.ExpressionType) {
  case ExprType::exprValue:
    code.values.emplace_back(static_cast<const exprValue &>(expr).value);
    emit(Action::Push_Value, code.values.size() - 1);
    return;
  case ExprType::Binary: {
    const Binary &binary = static_cast<const Binary &>(expr);
    GenerateExpression(*binary.left);
    GenerateExpression(*binary.right);
    emit(utils::getOperatorAction(static_cast<const Binary &>(expr).op));
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
      emit(Action::Pop);
      break;
    default:
      break;
    }
  }
  return code;
}
