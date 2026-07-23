#include "analyzer.h"
#include "AST.h"
#include "analyzer_variable.h"
#include "utils.h"

void Analyzer::AnalyzeExpression(const Expression &expr) {
  switch (expr.ExpressionType) {
  case ExprType::Variable: {
    if (!ignoreVariables &&
        !env->exists(static_cast<const Variable &>(expr).name))
      return errors.push_back(
          SemanticError("Undefined variable", expr.location));
    return;
  }
  case ExprType::Assignment:
    AnalyzeAssignment(static_cast<const Assignment &>(expr));
    break;
  case ExprType::Binary:
    AnalyzeBinary(static_cast<const Binary &>(expr));
    return;
  case ExprType::Unary:
    AnalyzeUnary(static_cast<const Unary &>(expr));
    return;
  case ExprType::Cast:
    return AnalyzeExpression(*static_cast<const Cast &>(expr).expr);
  case ExprType::exprValue:
    return;
  case ExprType::FunctionCall: {
    const auto &a = static_cast<const FunctionCall &>(expr);
    if (!env->exists(a.name))
      errors.push_back(
          SemanticError("Such function does not seem to exist", a.location));
    for (auto &expr : a.parameters) {
      AnalyzeExpression(*expr);
    }
    return;
  }
  default:
    errors.emplace_back("Unrecongnized Expression", expr.location);
  }
}

void Analyzer::AnalyzeBinary(const Binary &expr) {
  AnalyzeExpression(*expr.left);
  return AnalyzeExpression(*expr.right);
}

void Analyzer::AnalyzeAssignment(const Assignment &expr) {
  if (expr.left->ExpressionType != ExprType::Variable)
    errors.push_back(
        SemanticError("The definition operator can only be used to variables",
                      expr.location));
  AnalyzeExpression(*expr.right);
  env->Define(currentmodifiers, static_cast<const Variable &>(*expr.left).name,
              errors, expr.location);
}

void Analyzer::AnalyzeUnary(const Unary &expr) {
  switch (expr.op) {
  case Operator::PreIncr:
  case Operator::PostIncr:
    if (expr.expr->ExpressionType != ExprType::Variable) {
      return errors.push_back(
          SemanticError("The increment operator can be only used to variables",
                        expr.location));
    }
    AnalyzeExpression(*expr.expr);
    env->exists(static_cast<const Variable &>(*expr.expr).name)
        ->isallowed(MOD_NONE, errors, expr.location);
    return;
  case Operator::PreDecr:
  case Operator::PostDecr:
    if (expr.expr->ExpressionType != ExprType::Variable) {
      return errors.push_back(
          SemanticError("The decrement operator can be only used to variables",
                        expr.location));
    }
    AnalyzeExpression(*expr.expr);
    env->exists(static_cast<const Variable &>(*expr.expr).name)
        ->isallowed(MOD_NONE, errors, expr.location);
    return;
  default:
    return AnalyzeExpression(*expr.expr);
  }
}

void Analyzer::AnalyzeInput(const Input &stmt) {
  if (stmt.input->ExpressionType != ExprType::Variable &&
      stmt.input->ExpressionType != ExprType::Cast)
    return errors.push_back(
        SemanticError("Expressions cannot be used in the input function",
                      stmt.input->location));
  if (stmt.input->ExpressionType == ExprType::Cast) {
    const auto &a = static_cast<const Cast &>(*stmt.input);
    if (a.expr->ExpressionType != ExprType::Variable)
      return errors.push_back(
          SemanticError("The variable inside the cast operation is expected",
                        a.expr->location));
  }
}

void Analyzer::AnalyzeOutput(const Output &stmt) {
  return AnalyzeExpression(*stmt.output);
}

void Analyzer::AnalyzeExpressionStmt(const ExpressionStmt &stmt) {
  if (!utils::CheckModifiers(*stmt.expr, stmt.mods))
    errors.emplace_back("Invalid Modifier(s)", stmt.location);
  return AnalyzeExpression(*stmt.expr);
}

void Analyzer::AnalyzeWhile(const While &stmt) {
  AnalyzeExpression(*stmt.expr);
  bool prev = insideLoop;
  insideLoop = true;
  newScope();
  analyze(*stmt.Instructions);
  removeScope();
  insideLoop = prev;
}

void Analyzer::AnalyzeIf(const IfStatement &stmt) {
  if (stmt.expr)
    AnalyzeExpression(*stmt.expr);
  newScope();
  analyze(*stmt.Instructions);
  removeScope();
  if (stmt.elseStatement)
    AnalyzeIf(*stmt.elseStatement);
}

void Analyzer::AnalyzeFor(const For &stmt) {
  bool prev = insideLoop;
  insideLoop = true;
  newScope();
  env->Define(0, stmt.iterator, errors, stmt.location);
  auto iterator = env->exists(stmt.iterator);
  if (!utils::isDynamic(stmt.mods))
    iterator->lock();
  if (stmt.Initialvalue)
    AnalyzeExpression(*stmt.Initialvalue);
  AnalyzeExpression(*stmt.Finalvalue);
  if (stmt.step)
    AnalyzeExpression(*stmt.step);
  analyze(*stmt.Instructions);
  if (!utils::isDynamic(stmt.mods))
    iterator->unlock();
  removeScope();
  insideLoop = prev;
}

void Analyzer::AnalyzeBreak(const BreakStmt &stmt) {
  if (!insideLoop)
    errors.emplace_back("Cannot use break outside of the loop", stmt.location);
}

void Analyzer::AnalyzeContinue(const ContinueStmt &stmt) {
  if (!insideLoop)
    errors.emplace_back("Cannot use continue outside of the loop",
                        stmt.location);
}

void Analyzer::AnalyzeFunction(const FunctionStatement &stmt) {
  int8_t previous = 0;
  if (!env->parent && !env->exists(stmt.name))
    currentmodifiers |= MOD_GLOBAL;
  env->Define(currentmodifiers, stmt.name, errors, stmt.location);
  if (utils::isDynamic(env->exists(stmt.name)->mods)) {
    previous |= ignoreVariables;
    ignoreVariables = true;
  }
  previous |= insidefunction << 1;
  insidefunction = true;
  newScope();
  for (size_t i = 0; i < stmt.params.size(); i++) {
    if (!utils::CheckModifiers(stmt.params[i]))
      errors.emplace_back("Parameter number " + std::to_string(i + 1) +
                              " : Such modifier(s) cannot be used with the "
                              "parameters of the function",
                          stmt.location);
    env->Define(stmt.params[i].mods & utils::AllowedParam, stmt.params[i].name,
                errors, stmt.location);
  }
  analyze(*stmt.Instructions);
  insidefunction = previous & 0b10;
  ignoreVariables = previous & 0b01;
  removeScope();
}

void Analyzer::AnalyzeReturn(const ReturnStatement &stmt) {
  AnalyzeExpression(*stmt.expr);
  if (!insidefunction)
    errors.emplace_back("The return must be used inside the function",
                        stmt.location);
}

void Analyzer::AnalyzeBlock(const BlockStatement &stmt) {
  newScope();
  analyze(*stmt.instructions);
  removeScope();
}

signed char Analyzer::analyze(const Program &program) {
  for (const auto &stmt : program.statements) {
    currentmodifiers = stmt->mods;
    if (!utils::CheckModifiers(stmt->StatementType, stmt->mods))
      errors.emplace_back("The modifier(s) cannot be used to such statement",
                          stmt->location);
    switch (stmt->StatementType) {
    case StmtType::Output:
      AnalyzeOutput(static_cast<const Output &>(*stmt));
      break;
    case StmtType::Input:
      AnalyzeInput(static_cast<const Input &>(*stmt));
      break;
    case StmtType::ExpressionStmt:
      AnalyzeExpressionStmt(static_cast<const ExpressionStmt &>(*stmt));
      break;
    case StmtType::IfStatement:
      AnalyzeIf(static_cast<const IfStatement &>(*stmt));
      break;
    case StmtType::While:
      AnalyzeWhile(static_cast<const While &>(*stmt));
      break;
    case StmtType::For:
      AnalyzeFor(static_cast<const For &>(*stmt));
      break;
    case StmtType::BreakStmt:
      AnalyzeBreak(static_cast<const BreakStmt &>(*stmt));
      break;
    case StmtType::ContinueStmt:
      AnalyzeContinue(static_cast<const ContinueStmt &>(*stmt));
      break;
    case StmtType::FunctionStatement:
      AnalyzeFunction(static_cast<const FunctionStatement &>(*stmt));
      break;
    case StmtType::ReturnStatement:
      AnalyzeReturn(static_cast<const ReturnStatement &>(*stmt));
      break;
    case StmtType::BlockStatement:
      AnalyzeBlock(static_cast<const BlockStatement &>(*stmt));
      break;
    default:
      errors.push_back(SemanticError("Unknown Statement type", stmt->location));
    }
  }
  if (errors.size()) {
    return AnalyzerError;
  }
  return AnalyzerOK;
}

void Analyzer::printErrors() {
  std::cerr << "Semantic Error(s):\n";
  for (auto &err : errors) {
    std::cerr << "Line " << err.loc.line;
    std::cerr << ", column " << err.loc.column;
    std::cerr << ": " + err.err + ".\n";
  }
}

void Analyzer::newScope() { env = new AnalyzerEnv({}, env); }

void Analyzer::removeScope() {
  auto envcopy = env->parent;
  delete env;
  env = envcopy;
}
