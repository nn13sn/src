#pragma once
#include "AST.h"
#include "analyzer_env.h"
#include "semantic_error.h"
#define AnalyzerOK 0
#define AnalyzerError -1

class Analyzer {
  std::vector<SemanticError> errors = {};
  AnalyzerEnv *env = new AnalyzerEnv;
  bool insidefunction = false;
  int32_t currentmodifers = 0;
  bool ignoreVariables = false;
  void AnalyzeExpression(const Expression &expr);
  void AnalyzeBinary(const Binary &expr);
  void AnalyzeUnary(const Unary &expr);
  void AnalyzeInput(const Input &stmt);
  void AnalyzeOutput(const Output &stmt);
  void AnalyzeExpressionStmt(const ExpressionStmt &stmt);
  void AnalyzeWhile(const While &stmt);
  void AnalyzeFor(const For &stmt);
  void AnalyzeIf(const IfStatement &stmt);
  void AnalyzeFunction(const FunctionStatement &stmt);
  void AnalyzeReturn(const ReturnStatement &stmt);
  void AnalyzeBlock(const BlockStatement &stmt);
  void newScope();
  void removeScope();

public:
  void printErrors();
  signed char analyze(const Program &program);
};
