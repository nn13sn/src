#pragma once
#include "analyzer_variable.h"
#include "utils.h"
#include <string>
#include <unordered_map>

struct AnalyzerEnv {
  std::unordered_map<std::string, AnalyzerVariable> variables = {};
  inline static std::unordered_map<std::string, AnalyzerVariable> globals = {};
  AnalyzerEnv *parent = nullptr;
  AnalyzerVariable *exists(const std::string &name);
  void Define(const int32_t &mods, const std::string &name,
              std::vector<SemanticError> &errors, const Location &loc) {
    if (auto a = exists(name)) {
      a->isallowed(mods, errors, loc);
      return;
    }
    if (utils::isGlobal(mods))
      globals.insert({name, AnalyzerVariable(mods)});
    else
      variables.insert({name, AnalyzerVariable(mods)});
  }
};

inline AnalyzerVariable *AnalyzerEnv::exists(const std::string &name) {
  if (auto a = globals.find(name); a != globals.end())
    return &a->second;
  if (auto a = variables.find(name); a != variables.end())
    return &a->second;
  if (parent)
    return parent->exists(name);
  return nullptr;
}
