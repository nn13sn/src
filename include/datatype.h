#pragma once
#include <cstdint>
#include <string>
using dc_int = int64_t;
using dc_double = double;
using dc_char = char32_t;
using dc_bool = bool;
using dc_string = std::string;

enum class Datatype : uint8_t {
  Int,
  Double,
  Char,
  Bool,
  String,
  Array,
  Function,
  Invalid
};
