#include "runtime_operations.h"
#include "runtime_cast.h"
#include "vm_error.h"

RuntimeValue RuntimeOperations::Add(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"+\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Int, RuntimeCast::As<dc_int>(left) +
                                           RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Double,
                        RuntimeCast::As<dc_double>(left) +
                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Sub(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"-\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Int, RuntimeCast::As<dc_int>(left) -
                                           RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Double,
                        RuntimeCast::As<dc_double>(left) -
                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Mul(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"*\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Int, RuntimeCast::As<dc_int>(left) *
                                           RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Double,
                        RuntimeCast::As<dc_double>(left) *
                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Div(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"/\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int: {
    auto casted_right = RuntimeCast::As<dc_int>(right);
    if (casted_right == 0)
      throw VM_error("Division by 0 is not allowed");
    return RuntimeValue(Datatype::Int,
                        RuntimeCast::As<dc_int>(left) / casted_right);
  }
  case Datatype::Double: {
    auto casted_right = RuntimeCast::As<dc_double>(right);
    if (casted_right == 0)
      throw VM_error("Division by 0 is not allowed");
    return RuntimeValue(Datatype::Double,
                        RuntimeCast::As<dc_double>(left) / casted_right);
  }
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Mod(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"%\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int: {
    auto casted_right = RuntimeCast::As<dc_int>(right);
    if (casted_right == 0)
      throw VM_error("Division by 0 is not allowed");
    return RuntimeValue(Datatype::Int,
                        RuntimeCast::As<dc_int>(left) % casted_right);
  }
  case Datatype::Double:
    throw VM_error("Mod operator cannot be used with double");
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Equal(const RuntimeValue &left,
                                      const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"==\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) ==
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) ==
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::NotEqual(const RuntimeValue &left,
                                         const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"!=\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) !=
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) !=
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Less(const RuntimeValue &left,
                                     const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"<\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) <
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) <
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Greater(const RuntimeValue &left,
                                        const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \">\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) >
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) >
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::LessEq(const RuntimeValue &left,
                                       const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"<=\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) <=
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) <=
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::GreaterEq(const RuntimeValue &left,
                                          const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \">=\" cannot be used with such data type");
  auto common = RuntimeCast::CommonType(left.getType(), right.getType());
  switch (common) {
  case Datatype::Int:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_int>(left) >=
                                            RuntimeCast::As<dc_int>(right));
  case Datatype::Double:
    return RuntimeValue(Datatype::Bool, RuntimeCast::As<dc_double>(left) >=
                                            RuntimeCast::As<dc_double>(right));
  default:
    throw VM_error("Internal Error"); // unreachable
  }
}

RuntimeValue RuntimeOperations::Neg(RuntimeValue value) {
  std::visit(
      [&](const auto &a) {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, dc_int> || std::is_same_v<T, double>) {
          value.setData(a * (-1));
        } else
          throw VM_error(
              "A non-numeric data type cannot be used with negative operator");
      },
      value.getData());
  return value;
}

RuntimeValue RuntimeOperations::Not(const RuntimeValue &value) {
  RuntimeValue newValue;
  if (utils::isNumerical(value)) {
    return std::visit(
        [&](const auto &a) -> RuntimeValue {
          using T = std::decay_t<decltype(a)>;
          if constexpr (std::is_same_v<T, dc_int> ||
                        std::is_same_v<T, dc_double> ||
                        std::is_same_v<T, dc_char> ||
                        std::is_same_v<T, dc_bool>) {
            return RuntimeValue(Datatype::Bool, a == 0);
          } else
            throw VM_error("Internal Error"); // unreachable
        },
        value.getData());
  } else
    throw VM_error(
        "A non-numeric data type cannot be used with logical Not operator");
}
