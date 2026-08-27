#pragma once
#include "location.h"
#include "semantic_error.h"
#include "slot_table.h"
#include <cstdint>
#include <vector>
struct AnalyzerVariable {
  uint32_t mods = MOD_NONE;
  uint32_t locked = 0;
  VariableID ID;
  inline void lock() { locked++; }
  inline void unlock() { locked--; }
  inline bool isallowed(const uint32_t &mods,
                        std::vector<SemanticError> &errors,
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
  AnalyzerVariable(const uint32_t &modifiers, const VariableID &ID)
      : mods(modifiers), ID(ID) {};
};
