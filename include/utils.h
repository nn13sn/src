#pragma once
#include "AST.h"
#include "runtime_value.h"
#include "tokens.h"
#include <array>
class RuntimeValue;
// just some short helper functions here
namespace utils {
bool isNumerical(const RuntimeValue &value);

bool isLogical(const Operator &op);

bool isArrow(const Operator &op);

bool isExpression(const TokenType &type);

bool CheckModifiers(const StmtType &type, const int32_t &modifiers);

bool CheckModifiers(const Expression &expr, const int32_t &modifiers);

bool isGlobal(const int32_t &modifiers);

bool isDynamic(const int32_t &modifiers);

} // namespace utils
