#pragma once
#include "AST.h"
#include "action.h"
#include "runtime_value.h"
#include "tokens.h"
#include <array>
class RuntimeValue;
// just some short helper functions here & check functions as well
namespace utils {

constexpr std::array<int32_t, static_cast<size_t>(StmtType::Amount)>
    AllowedStmt{
        0b000, // Input
        0b000, // Output
        0b101, // Expression
        0b000, // If statement
        0b000, // While
        0b010, // For
        0b000, // Break
        0b000, // Continue
        0b111, // Function
        0b000, // Return
        0b000  // Block
    };

const int32_t AllowedDef = 0b101;

const int32_t AllowedParam = 0b100;

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

Action getOperatorAction(const Operator &op);

} // namespace utils
