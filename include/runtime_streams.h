#pragma once
#include "runtime_value.h"
#include "utils.h"
namespace RuntimeStreams {
void Print(const RuntimeValue &value);
void PrintValue(const dc_int &value);
void PrintValue(const dc_double &value);
void PrintValue(const dc_char &value);
void PrintValue(const dc_bool &value);
void PrintValue(const dc_string &value);
void PrintValue(const auto &);

RuntimeValue ReadValue(const Datatype &type);
template <typename Type> Type Read() {
  Type value;
  std::cin >> value;
  return value;
}
template <> dc_char Read<dc_char>();
} // namespace RuntimeStreams
