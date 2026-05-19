#pragma once
#include "location.h"
#include "semantic_error.h"
#include <cstdint>
#include <vector>
struct AnalyzerVariable {
  int32_t mods = MOD_NONE;
  int32_t locked = 0;
  inline void lock() { locked++; }
  inline void unlock() { locked--; }
  inline bool isallowed(const int32_t &mods, std::vector<SemanticError> &errors,
                        const Location &loc) {
    if (this->mods & MOD_CONST)
      errors.emplace_back("A constant variable (or function) cannot be changed",
                          loc);
    if (locked)
      errors.emplace_back("An iterator cannot be changed", loc);
    if (mods)
      errors.emplace_back("Modifiers can only be used with the first "
                          "declaration of the variable",
                          loc);
    return false;
  }
  AnalyzerVariable(const int32_t &modifiers) : mods(modifiers) {};
};
