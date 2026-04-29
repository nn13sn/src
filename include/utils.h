#pragma once
#include "AST.h"
#include "interpreter.h"
#include "lexer.h"
// just some short helper functions here
namespace utils {
inline bool isNumerical(const RuntimeValue &value) {
  if (value.type == Datatype::Int || value.type == Datatype::Char ||
      value.type == Datatype::Double || value.type == Datatype::Bool)
    return true;
  return false;
}

inline bool isLogical(const Operator &op) {
  if (op == Operator::Greater || op == Operator::GreaterEq ||
      op == Operator::Less || op == Operator::LessEq || op == Operator::Equal ||
      op == Operator::NotEqual)
    return true;
  return false;
}

inline bool isArrow(const Operator &op) {
  if (op == Operator::Arrow || op == Operator::ArrowEq)
    return true;
  return false;
}

inline bool isExpression(const TokenType &type) {
  if (type == TokenType::Identifier || type == TokenType::Operator ||
      type == TokenType::Separator || type == TokenType::String ||
      type == TokenType::Boolean || type == TokenType::Number ||
      type == TokenType::Symbol || type == TokenType::Double)
    return true;
  return false;
}

inline bool CheckModifiers(const StmtType &type, const int32_t &modifiers) {
  static constexpr std::array<int32_t, static_cast<size_t>(StmtType::Amount)>
      AllowedStmt{0b00, 0b00, 0b11, 0b00, 0b00, 0b10, 0b11, 0b00, 0b00};
  if (modifiers & ~AllowedStmt[static_cast<size_t>(type)])
    // the idea of idea of creating a table of allowed is mine,
    // but I had to research to get the such if statement here
    return false;
  return true;
}

inline bool CheckModifiers(const Expression &expr, const int32_t &modifiers) {
  static constexpr int32_t AllowedDef = 0b01;
  if (expr.ExpressionType == ExprType::Binary &&
      static_cast<const Binary &>(expr).op == Operator::Def) {
    if (modifiers & ~AllowedDef)
      return false;
    return true;
  }
  if (modifiers == MOD_NONE)
    return true;
  return false;
}

inline bool isGlobal(const int32_t &modifiers) {
  return (modifiers & MOD_GLOBAL) ? true : false;
}

inline bool isDynamic(const int32_t &modifiers) {
  return (modifiers & MOD_DYNAMIC) ? true : false;
}

} // namespace utils
