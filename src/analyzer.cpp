#include "analyzer.h"
#include "AST.h"
#include "lexer.h"

SemanticError::SemanticError(const std::string &err, const Location &loc) {
  this->err = err;
  this->loc = loc;
}

bool AnalyzerEnv::exists(const std::string &name) {
  if (globals.find(name) != globals.end())
    return true;
  if (variables.find(name) != variables.end())
    return true;
  if (parent)
    return parent->exists(name);
  return false;
}

void Analyzer::AnalyzeExpression(const Expression &expr) {
  switch (expr.ExpressionType) {
  case ExprType::Variable: {
    if (!env->exists(static_cast<const Variable &>(expr).name))
      return errors.push_back(
          SemanticError("Undefined variable", expr.location));
    return;
  }
  case ExprType::Binary: {
    const auto &a = static_cast<const Binary &>(expr);
    if (a.op == Operator::Def) {
      if (a.left->ExpressionType != ExprType::Variable)
        errors.push_back(SemanticError(
            "The definition operator can only be used to variables",
            a.location));
      AnalyzeExpression(*a.right);
      if (isGlobal) {
        if (!env->exists(static_cast<const Variable &>(*a.left).name))
          env->globals.insert(static_cast<const Variable &>(*a.left).name);
        else
          errors.emplace_back("The variable was already declared before",
                              a.left->location);
      } else
        env->variables.insert(static_cast<const Variable &>(*a.left).name);
      return;
    }
    AnalyzeExpression(*a.left);
    return AnalyzeExpression(*a.right);
  }
  case ExprType::Unary: {
    const auto &a = static_cast<const Unary &>(expr);
    switch (a.op) {
    case Operator::PreIncr:
    case Operator::PostIncr:
      if (a.expr->ExpressionType != ExprType::Variable) {
        return errors.push_back(SemanticError(
            "The increment operator can be only used to variables",
            a.location));
      }
      return AnalyzeExpression(*a.expr);
    case Operator::PreDecr:
    case Operator::PostDecr:
      if (a.expr->ExpressionType != ExprType::Variable) {
        return errors.push_back(SemanticError(
            "The decrement operator can be only used to variables",
            a.location));
      }
      return AnalyzeExpression(*a.expr);
    default:
      return AnalyzeExpression(*a.expr);
    }
  }
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
  env->variables.insert(stmt.iterator);
  if (stmt.Initialvalue)
    AnalyzeExpression(*stmt.Initialvalue);
  AnalyzeExpression(*stmt.Finalvalue);
  if (stmt.step)
    AnalyzeExpression(*stmt.step);
  analyze(*stmt.Instructions);
  removeScope();
}

void Analyzer::AnalyzeFunction(const FunctionStatement &stmt) {
  if (env->parent && !isGlobal)
    env->variables.insert(stmt.name);
  else
    env->globals.insert(stmt.name);
  bool previous = insidefunction;
  insidefunction = true;
  newScope();
  for (auto &par : stmt.parameters) {
    env->variables.insert(par);
  }
  analyze(*stmt.Instructions);
  insidefunction = previous;
  removeScope();
}

void Analyzer::AnalyzeReturn(const ReturnStatement &stmt) {
  AnalyzeExpression(*stmt.expr);
  if (!insidefunction)
    errors.emplace_back("The return must be used inside the function",
                        stmt.location);
}

void Analyzer::AnalyzeGlobal(const Global &stmt) {
  isGlobal = true;
  if (stmt.stmt->StatementType == StmtType::ExpressionStmt) {
    const auto &expr = *static_cast<const ExpressionStmt &>(*stmt.stmt).expr;
    if (expr.ExpressionType == ExprType::Binary) {
      const auto &bin = static_cast<const Binary &>(expr);
      if (bin.op == Operator::Def) {
        AnalyzeExpression(bin);
      } else
        errors.emplace_back("A variable defintion is expected", stmt.location);
    } else
      errors.emplace_back("A variable defintion is expected", stmt.location);
  } else if (stmt.stmt->StatementType == StmtType::FunctionStatement) {
    AnalyzeFunction(static_cast<const FunctionStatement &>(*stmt.stmt));
  } else
    errors.emplace_back("A defintion is expected", stmt.location);
  isGlobal = false;
}

void Analyzer::AnalyzeBlock(const BlockStatement &stmt) {
  newScope();
  analyze(*stmt.instructions);
  removeScope();
}

signed char Analyzer::analyze(const Program &program) {
  for (const auto &stmt : program.statements) {
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
    case StmtType::Global:
      AnalyzeGlobal(static_cast<const Global &>(*stmt));
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
