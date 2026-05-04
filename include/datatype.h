#pragma once
#include <cstdint>

enum class Datatype : uint8_t {
  Int,
  Char,
  String,
  Double,
  Bool,
  Array,
  Function,
  Invalid
};
