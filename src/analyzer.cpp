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
  if (expr.op == Operator::Def) {
    if (expr.left->ExpressionType != ExprType::Variable)
      errors.push_back(
          SemanticError("The definition operator can only be used to variables",
                        expr.location));
    AnalyzeExpression(*expr.right);
    if (utils::isGlobal(currentmodifers)) {
      if (!env->exists(static_cast<const Variable &>(*expr.left).name))
        env->globals.insert(
            std::make_pair(static_cast<const Variable &>(*expr.left).name,
                           AnalyzerVariable()));
      else
        errors.emplace_back("The variable was already declared before",
                            expr.left->location);
    } else {
      if (auto a = env->exists(static_cast<const Variable &>(*expr.left).name);
          a) {
        if (!a->isallowed())
          errors.emplace_back("Cannot re-define a constant variable",
                              expr.left->location);
        return;
      }
      auto var = AnalyzerVariable();
      if (utils::isConst(currentmodifers))
        var.isConst = true;
      env->variables.insert(
          {static_cast<const Variable &>(*expr.left).name, var});
    }
    return;
  }
  AnalyzeExpression(*expr.left);
  return AnalyzeExpression(*expr.right);
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
    if (!env->exists(static_cast<const Variable &>(*expr.expr).name)
             ->isallowed())
      errors.emplace_back(
          "The increment operator cannot be used to constant variables",
          expr.expr->location);
    return;
  case Operator::PreDecr:
  case Operator::PostDecr:
    if (expr.expr->ExpressionType != ExprType::Variable) {
      return errors.push_back(
          SemanticError("The decrement operator can be only used to variables",
                        expr.location));
    }
    AnalyzeExpression(*expr.expr);
    if (!env->exists(static_cast<const Variable &>(*expr.expr).name)
             ->isallowed())
      errors.emplace_back(
          "The decrement operator cannot be used to constant variables",
          expr.expr->location);
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
      return errors.push_back(SemanticError(
          "The variable must be inside the cast operation", a.expr->location));
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
  newScope();
  analyze(*stmt.Instructions);
  removeScope();
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
  newScope();
  if (auto a = env->exists(stmt.iterator)) {
    if (!a->isallowed())
      errors.emplace_back(
          "Cannot use a constant or locked variable as an iterator",
          stmt.location);
  } else
    env->variables.insert({stmt.iterator, AnalyzerVariable()});
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
}

void Analyzer::AnalyzeFunction(const FunctionStatement &stmt) {
  int8_t previous = 0;
  if (auto a = env->exists(stmt.name); a && !a->isallowed()) {
    errors.emplace_back("Cannot re-define a constant function", stmt.location);
    return;
  }
  auto var = AnalyzerVariable();
  if (utils::isConst(stmt.mods))
    var.isConst = true;
  if (env->parent && !utils::isGlobal(currentmodifers))
    env->variables.insert({stmt.name, var});
  else
    env->globals.insert({stmt.name, var});
  if (utils::isDynamic(currentmodifers)) {
    previous |= ignoreVariables;
    ignoreVariables = true;
  }
  previous |= insidefunction << 1;
  insidefunction = true;
  newScope();
  for (auto &par : stmt.parameters) {
    env->variables.insert({par, AnalyzerVariable()});
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
    currentmodifers = stmt->mods;
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
