#pragma once
#include <cstdint>
enum class Action : uint8_t {
  Push_Value,

  Add,
  Sub,
  Mul,
  Div,

  Neg,

  Pop
};
