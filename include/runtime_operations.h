#pragma once
#include "runtime_value.h"
namespace RuntimeOperations {
RuntimeValue Add(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Sub(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Mul(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Div(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Mod(const RuntimeValue &left, const RuntimeValue &right);

RuntimeValue Equal(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue NotEqual(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Greater(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue Less(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue GreaterEq(const RuntimeValue &left, const RuntimeValue &right);
RuntimeValue LessEq(const RuntimeValue &left, const RuntimeValue &right);

RuntimeValue Neg(RuntimeValue value);
RuntimeValue Not(const RuntimeValue &value);
RuntimeValue PreIncr(const RuntimeValue &value);
RuntimeValue PostIncr(const RuntimeValue &value);
RuntimeValue PreDecr(const RuntimeValue &value);
RuntimeValue PostDecr(const RuntimeValue &value);
} // namespace RuntimeOperations
