#pragma once
#include <cstdint>
enum class Operator : uint8_t {
  Add,
  Sub,
  Mul,
  Div,
  Mod,

  PreIncr,
  PostIncr,
  PreDecr,
  PostDecr,
  Not,

  Less,
  Greater,
  LessEq,
  GreaterEq,
  Equal,
  NotEqual,
  AND,
  OR,

  Def,
  Arrow,
  ArrowEq,
  Invalid
};
