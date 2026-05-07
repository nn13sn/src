#pragma once
#include "runtime_value.h"
#include "runtime_variable.h"
#include <unordered_map>
struct Environment {
  std::unordered_map<std::string, RuntimeVariable> values = {};
  inline static std::unordered_map<std::string, RuntimeVariable> globals = {};
  std::shared_ptr<Environment> parent = nullptr;
  RuntimeVariable get(const std::string &name);
  RuntimeVariable *getPointer(const std::string &name);
  void set(const std::string &name, const RuntimeValue &value,
           const Location &loc, const int32_t &mods);
  bool newGlobal(const std::string &name, const RuntimeValue &value);
};
