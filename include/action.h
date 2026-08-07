#pragma once
#include <cstdint>
enum class Action : uint8_t {
  Push_Value,

  Store_Local,
  Load_Local,

  Add,
  Sub,
  Mul,
  Div,
  Mod,

  Equal,
  NotEqual,
  Greater,
  Less,
  LessEq,
  GreaterEq,

  Cast,

  Neg,
  Not,

  Pop,

  Print
};
