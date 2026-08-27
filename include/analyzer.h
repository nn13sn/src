#pragma once
#include "AST.h"
#include "analyzer_env.h"
#include "semantic_error.h"
#include "slot_table.h"
#define AnalyzerOK 0
#define AnalyzerError -1

class Analyzer {
  std::vector<SemanticError> errors = {};
  std::vector<uint32_t> Scopestats;
  AnalyzerEnv *env = new AnalyzerEnv(&table);
  bool insidefunction = false;
  bool insideLoop = false;
  int32_t currentmodifiers = 0;
  bool ignoreVariables = false;

  void AnalyzeExpression(const Expression &expr);
  void AnalyzeBinary(const Binary &expr);
  void AnalyzeAssignment(const Assignment &expr);
  void AnalyzeUnary(const Unary &expr);
  void AnalyzeInput(const Input &stmt);
  void AnalyzeOutput(const Output &stmt);
  void AnalyzeExpressionStmt(const ExpressionStmt &stmt);
  void AnalyzeWhile(const While &stmt);
  void AnalyzeFor(const For &stmt);
  void AnalyzeBreak(const BreakStmt &stmt);
  void AnalyzeContinue(const ContinueStmt &stmt);
  void AnalyzeIf(const IfStatement &stmt);
  void AnalyzeFunction(const FunctionStatement &stmt);
  void AnalyzeReturn(const ReturnStatement &stmt);
  void AnalyzeBlock(const BlockStatement &stmt);
  void newScope();
  void removeScope();

public:
  void printErrors();
  signed char analyze(const Program &program);
  Slot_Table table;
  ~Analyzer() { delete env; }
};
