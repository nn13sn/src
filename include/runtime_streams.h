#pragma once
#include "runtime_value.h"
namespace RuntimeStreams {
void Print(const RuntimeValue &value);
void PrintValue(const dc_int &value);
void PrintValue(const dc_double &value);
void PrintValue(const dc_char &value);
void PrintValue(const dc_bool &value);
void PrintValue(const dc_string &value);
void PrintValue(const auto &);
} // namespace RuntimeStreams
