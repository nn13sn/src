#pragma once
#include "analyzer_variable.h"
#include <string>
#include <unordered_map>

struct AnalyzerEnv {
  std::unordered_map<std::string, AnalyzerVariable> variables = {};
  inline static std::unordered_map<std::string, AnalyzerVariable> globals = {};
  AnalyzerEnv *parent = nullptr;
  AnalyzerVariable *exists(const std::string &name);
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
