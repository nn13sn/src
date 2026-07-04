#pragma once
#include "interpreter.h"
// RAII scopes for interpreter
struct Scope {
  Interpreter &interpreter;
  inline Scope(Interpreter &interpreter) : interpreter(interpreter) {
    interpreter.addScope();
  }
  inline ~Scope() { interpreter.popScope(); }
};
