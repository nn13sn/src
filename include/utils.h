#pragma once
#include "lexer.h"
#include "runtime_value.h"
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
} // namespace utils
