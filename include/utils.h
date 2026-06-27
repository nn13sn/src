#pragma once
#include "AST.h"
#include "runtime_value.h"
#include "tokens.h"
#include <array>
class RuntimeValue;
// just some short helper functions here
namespace utils {

static constexpr std::array<int32_t, static_cast<size_t>(StmtType::Amount)>
    AllowedStmt{0b000, 0b000, 0b101, 0b000, 0b000, 0b010, 0b111, 0b000, 0b000};

static constexpr int32_t AllowedDef = 0b101;

static constexpr int32_t AllowedParam = 0b100;

bool isNumerical(const RuntimeValue &value);

bool isLogical(const Operator &op);

bool isArrow(const Operator &op);

bool isExpression(const TokenType &type);

bool isModifier(const Token &token);

bool CheckModifiers(const StmtType &type, const int32_t &modifiers);

bool CheckModifiers(const Expression &expr, const int32_t &modifiers);

bool CheckModifiers(const Parameter &param);

bool isGlobal(const int32_t &modifiers);

bool isDynamic(const int32_t &modifiers);

bool isConst(const int32_t &modifiers);

} // namespace utils
