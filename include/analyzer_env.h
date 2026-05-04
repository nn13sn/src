#pragma once
#include "analyzer_variable.h"
#include <string>
#include <unordered_map>

struct AnalyzerEnv {
  std::unordered_map<std::string, AnalyzerVariable> variables = {};
  inline static std::unordered_map<std::string, AnalyzerVariable> globals = {};
  AnalyzerEnv *parent = nullptr;
  bool exists(const std::string &name);
};

inline bool AnalyzerEnv::exists(const std::string &name) {
  if (globals.find(name) != globals.end())
    return true;
  if (variables.find(name) != variables.end())
    return true;
  if (parent)
    return parent->exists(name);
  return false;
}
