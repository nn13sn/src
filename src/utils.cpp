#include "utils.h"
#include "runtime_value.h"
#include "tokens.h"
bool utils::isNumerical(const RuntimeValue &value) {
  if (value.getType() == Datatype::Int || value.getType() == Datatype::Char ||
      value.getType() == Datatype::Double || value.getType() == Datatype::Bool)
    return true;
  return false;
}

bool utils::isLogical(const Operator &op) {
  if (op == Operator::Greater || op == Operator::GreaterEq ||
      op == Operator::Less || op == Operator::LessEq || op == Operator::Equal ||
      op == Operator::NotEqual)
    return true;
  return false;
}

bool utils::isArrow(const Operator &op) {
  if (op == Operator::Arrow || op == Operator::ArrowEq)
    return true;
  return false;
}

bool utils::isExpression(const TokenType &type) {
  if (type == TokenType::Identifier || type == TokenType::Operator ||
      type == TokenType::Separator || type == TokenType::String ||
      type == TokenType::Boolean || type == TokenType::Number ||
      type == TokenType::Symbol || type == TokenType::Double)
    return true;
  return false;
}

bool utils::isModifier(const Token &token) {
  if (token.type != TokenType::Keyword)
    return false;
  auto keyword = static_cast<Keyword>(token.value);
  if (keyword == Keyword::Dynamic || keyword == Keyword::Global ||
      keyword == Keyword::Const)
    return true;
  return false;
}

bool utils::CheckModifiers(const StmtType &type, const int32_t &modifiers) {

  if (modifiers & ~AllowedStmt[static_cast<size_t>(type)])
    // the idea of creating a table of allowed is mine,
    // but I had to research to make such if statement here
    return false;
  return true;
}

bool utils::CheckModifiers(const Expression &expr, const int32_t &modifiers) {
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

bool utils::CheckModifiers(const Parameter &param) {
  if (param.mods & ~AllowedParam)
    return false;
  return true;
}

bool utils::isGlobal(const int32_t &modifiers) {
  return (modifiers & MOD_GLOBAL) ? true : false;
}

bool utils::isDynamic(const int32_t &modifiers) {
  return (modifiers & MOD_DYNAMIC) ? true : false;
}

bool utils::isConst(const int32_t &modifiers) {
  return (modifiers & MOD_CONST) ? true : false;
}

Action utils::getOperatorAction(const Operator &op, const bool &binary) {
  switch (op) {
  case Operator::Add:
    return Action::Add;
  case Operator::Div:
    return Action::Div;
  case Operator::Mul:
    return Action::Mul;
  case Operator::Sub:
    return (binary) ? Action::Sub : Action::Neg;
  case Operator::Equal:
    return Action::Equal;
  case Operator::NotEqual:
    return Action::NotEqual;
  case Operator::Less:
    return Action::Less;
  case Operator::Greater:
    return Action::Greater;
  case Operator::LessEq:
    return Action::LessEq;
  case Operator::GreaterEq:
    return Action::GreaterEq;
  case Operator::Not:
    return Action::Not;
  case Operator::PreIncr:
    return Action::PreIncr;
  case Operator::PostIncr:
    return Action::PostIncr;
  case Operator::PreDecr:
    return Action::PreDecr;
  case Operator::PostDecr:
    return Action::PostDecr;
  }
}

char32_t utils::decodeUTF8(const std::string &s) {
  const unsigned char *bytes =
      reinterpret_cast<const unsigned char *>(s.data());

  switch (s.size()) {
  case 1:
    return bytes[0];

  case 2:
    return ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);

  case 3:
    return ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) |
           (bytes[2] & 0x3F);

  case 4:
    return ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) |
           ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
  }
}

size_t utils::utf8CharLength(const unsigned char &c) {
  if ((c & 0x80) == 0x00)
    return 1;

  if ((c & 0xE0) == 0xC0)
    return 2;

  if ((c & 0xF0) == 0xE0)
    return 3;

  if ((c & 0xF8) == 0xF0)
    return 4;

  throw std::invalid_argument("Invalid UTF-8 character");
}
