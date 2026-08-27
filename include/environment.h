#pragma once
#include "runtime_value.h"
#include "runtime_variable.h"
struct Environment {
  std::vector<size_t> scopesizes;
  std::vector<RuntimeVariable> locals = {};
  inline static std::vector<RuntimeVariable> globals = {};
  std::shared_ptr<Environment> parent = nullptr;

  Environment(const uint32_t &size);
  void enterScope();
  void exitScope();
  RuntimeVariable get(const uint32_t &index);
  RuntimeVariable *getPointer(const uint32_t &index);
  void set(const uint32_t &index, const RuntimeValue &value,
           const int32_t &mods);
  bool newGlobal(const uint32_t &index, const RuntimeValue &value);
};
