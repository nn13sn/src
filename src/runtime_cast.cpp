#include "runtime_cast.h"
#include "vm_error.h"
#include <limits>
bool RuntimeCast::CanCast(const Datatype &from, const Datatype &to) {
  return AllowedCasts[static_cast<std::size_t>(from)] & getBit(to);
}

Datatype RuntimeCast::CommonType(const Datatype &left, const Datatype &right) {
  if (left == Datatype::Double || right == Datatype::Double)
    return Datatype::Double;
  return Datatype::Int;
}

RuntimeValue RuntimeCast::Cast(const RuntimeValue &value, const Datatype &to) {
  switch (to) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Int, As<dc_int>(value));
  case Datatype::Double:
    return RuntimeValue(Datatype::Double, As<dc_double>(value));
  case Datatype::Char:
    return RuntimeValue(Datatype::Char, As<dc_char>(value));
  case Datatype::Bool:
    return RuntimeValue(Datatype::Bool, As<dc_bool>(value));
  case Datatype::String:
    return RuntimeValue(Datatype::String, As<dc_string>(value));
  default:
    throw VM_error("Invalid data type to cast");
  }
}

template <> dc_int RuntimeCast::As<dc_int>(const RuntimeValue &value) {
  if (!CanCast(value.getType(), Datatype::Int))
    throw VM_error("Such data type cannot be casted to int");
  return std::visit(
      [&](const auto &a) -> dc_int {
        using Type = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<Type, dc_double>) {
          if (a < static_cast<dc_double>(std::numeric_limits<dc_int>::min()) ||
              a > static_cast<dc_double>(std::numeric_limits<dc_int>::max()))
            throw VM_error("Value is out of range to be casted to int");
          else
            return static_cast<dc_int>(a);
        } else if constexpr (std::is_same_v<Type, dc_int> ||
                             std::is_same_v<Type, dc_char> ||
                             std::is_same_v<Type, dc_bool>)
          return static_cast<dc_int>(a);
        else
          throw VM_error("Internal error"); // unreachable
      },
      value.getData());
}

template <> dc_double RuntimeCast::As<dc_double>(const RuntimeValue &value) {
  if (!CanCast(value.getType(), Datatype::Double))
    throw VM_error("Such data type cannot be casted to double");
  return std::visit(
      [&](const auto &a) -> dc_double {
        using Type = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<Type, dc_int> ||
                      std::is_same_v<Type, dc_double> ||
                      std::is_same_v<Type, dc_char> ||
                      std::is_same_v<Type, dc_bool>)
          return static_cast<dc_double>(a);
        else
          throw VM_error("Internal error"); // unreachable
      },
      value.getData());
}

template <> dc_char RuntimeCast::As<dc_char>(const RuntimeValue &value) {
  if (!CanCast(value.getType(), Datatype::Char))
    throw VM_error("Such data type cannot be casted to char");
  return std::visit(
      [&](const auto &a) -> dc_char {
        using Type = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<Type, dc_int>) {
          if (a < static_cast<dc_int>(std::numeric_limits<dc_char>::min()) ||
              a > static_cast<dc_int>(std::numeric_limits<dc_char>::max()))
            throw VM_error("The value is out of range to be casted to char");
          else
            return static_cast<dc_char>(a);
        } else if constexpr (std::is_same_v<Type, dc_double>) {
          if (a < static_cast<dc_double>(std::numeric_limits<dc_char>::min()) ||
              a > static_cast<dc_double>(std::numeric_limits<dc_char>::max()))
            throw VM_error("The value is out of range to be casted to char");
          else
            return static_cast<dc_char>(a);
        } else if constexpr (std::is_same_v<Type, dc_char> ||
                             std::is_same_v<Type, dc_bool>)
          return static_cast<dc_char>(a);
        else
          throw VM_error("Internal error"); // unreachable
      },
      value.getData());
}

template <> dc_bool RuntimeCast::As<dc_bool>(const RuntimeValue &value) {
  if (!CanCast(value.getType(), Datatype::Bool))
    throw VM_error("Such data type cannot be casted to bool");
  return std::visit(
      [&](const auto &a) -> dc_bool {
        using Type = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<Type, dc_int> ||
                      std::is_same_v<Type, dc_double> ||
                      std::is_same_v<Type, dc_char> ||
                      std::is_same_v<Type, dc_bool>)
          return static_cast<dc_bool>(a != 0);
        else
          throw VM_error("Internal error"); // unreachable
      },
      value.getData());
}

template <> dc_string RuntimeCast::As<dc_string>(const RuntimeValue &value) {
  if (!CanCast(value.getType(), Datatype::String))
    throw VM_error("Such data type cannot be casted to string");
  return std::visit(
      [&](const auto &a) -> dc_string {
        using Type = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<Type, dc_string>)
          return static_cast<dc_string>(a);
        else
          throw VM_error("Internal error"); // unreachable
      },
      value.getData());
}
