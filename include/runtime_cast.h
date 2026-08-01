#pragma once
#include "datatype.h"
#include "runtime_value.h"

namespace RuntimeCast {

constexpr uint32_t getBit(const Datatype &type) {
  return 1u << static_cast<uint32_t>(type);
}

constexpr uint32_t AllowedCasts[] = {
    // int
    getBit(Datatype::Int) | getBit(Datatype::Double) | getBit(Datatype::Char) |
        getBit(Datatype::Bool),

    // double
    getBit(Datatype::Int) | getBit(Datatype::Double) | getBit(Datatype::Char) |
        getBit(Datatype::Bool),

    // char
    getBit(Datatype::Int) | getBit(Datatype::Double) | getBit(Datatype::Char) |
        getBit(Datatype::Bool),

    // bool
    getBit(Datatype::Int) | getBit(Datatype::Double) | getBit(Datatype::Char) |
        getBit(Datatype::Bool),

    // string
    getBit(Datatype::String),

    // array
    getBit(Datatype::Array),

    // function
    getBit(Datatype::Function)};

bool CanCast(const Datatype &from, const Datatype &to);

Datatype CommonType(const Datatype &left, const Datatype &right);

RuntimeValue Cast(const RuntimeValue &value, const Datatype &to);

template <typename ReturnType> ReturnType As(const RuntimeValue &value);

template <> dc_int As<dc_int>(const RuntimeValue &value);

template <> dc_double As<dc_double>(const RuntimeValue &value);

template <> dc_bool As<dc_bool>(const RuntimeValue &value);

template <> dc_char As<dc_char>(const RuntimeValue &value);

template <> dc_string As<dc_string>(const RuntimeValue &value);
} // namespace RuntimeCast
