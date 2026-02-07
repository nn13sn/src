#pragma once
#include "AST.h"
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>

struct Interpreter{
  std::vector<std::unordered_map <std::string, int>> variables;
  int* findVar(const std::string& name);
  void execute(const Program& program);
  void matchStatement(const Statement& stmt);
  void declaration(const Declaration& stmt);
  void output(const Output& stmt);
  void definition(const Definition& stmt);
  int eval(const Expression& expr);
  void ifStatement(const IfStatement& stmt);
};
