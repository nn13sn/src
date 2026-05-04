#pragma once
#include "runtime_value.h"

struct RuntimeVariable {
  RuntimeValue value;
  RuntimeVariable(const RuntimeValue &val) : value(val) {};
  RuntimeVariable() {};
};
