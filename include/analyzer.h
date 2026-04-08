#pragma once
#include "AST.h"
#include <iostream>
#include <string>
#include <unordered_set>
#define AnalyzerOK 0
#define AnalyzerError -1
struct AnalyzerEnv {
  std::unordered_set<std::string> variables = {};
  AnalyzerEnv *parent = nullptr;
  bool exists(const std::string &name);
};

struct SemanticError {
  Location loc;
  std::string err;
  SemanticError(const std::string &err, const Location &loc);
};

class Analyzer {
  std::vector<SemanticError> errors = {};
  AnalyzerEnv *env = new AnalyzerEnv;
  bool insidefunction = false;
  void AnalyzeExpression(const Expression &expr);
  void AnalyzeInput(const Input &stmt);
  void AnalyzeOutput(const Output &stmt);
  void AnalyzeExpressionStmt(const ExpressionStmt &stmt);
  void AnalyzeWhile(const While &stmt);
  void AnalyzeFor(const For &stmt);
  void AnalyzeIf(const IfStatement &stmt);
  void AnalyzeFunction(const FunctionStatement &stmt);
  void AnalyzeReturn(const ReturnStatement &stmt);
  void newScope();
  void removeScope();

public:
  void printErrors();
  signed char analyze(const Program &program);
};
