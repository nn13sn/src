#include "runtime_operations.h"
#include "vm_error.h"
#include <stdexcept>

RuntimeValue RuntimeOperations::Add(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"+\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) +
                                         std::get<int64_t>(right.getData()));
}

RuntimeValue RuntimeOperations::Sub(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"-\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) -
                                         std::get<int64_t>(right.getData()));
}

RuntimeValue RuntimeOperations::Mul(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"*\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int, std::get<int64_t>(left.getData()) *
                                         std::get<int64_t>(right.getData()));
}

RuntimeValue RuntimeOperations::Div(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"/\" cannot be used with such data type");
  auto casted_right = std::get<int64_t>(right.getData());
  if (casted_right == 0)
    throw VM_error("Division by 0 is not possible.");
  return RuntimeValue(Datatype::Int,
                      std::get<int64_t>(left.getData()) / casted_right);
}

RuntimeValue RuntimeOperations::Mod(const RuntimeValue &left,
                                    const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"%\" cannot be used with such data type");
  auto casted_right = std::get<int64_t>(right.getData());
  if (casted_right == 0)
    throw VM_error("Division by 0 is not possible.");
  return RuntimeValue(Datatype::Int,
                      std::get<int64_t>(left.getData()) % casted_right);
}

RuntimeValue RuntimeOperations::Equal(const RuntimeValue &left,
                                      const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"==\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) ==
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::NotEqual(const RuntimeValue &left,
                                         const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"!=\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) !=
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::Less(const RuntimeValue &left,
                                     const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"<\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) <
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::Greater(const RuntimeValue &left,
                                        const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \">\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) >
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::LessEq(const RuntimeValue &left,
                                       const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \"<=\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) <=
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::GreaterEq(const RuntimeValue &left,
                                          const RuntimeValue &right) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw VM_error("Operator \">=\" cannot be used with such data type");
  return RuntimeValue(Datatype::Int,
                      static_cast<int64_t>(std::get<int64_t>(left.getData()) >=
                                           std::get<int64_t>(right.getData())));
}

RuntimeValue RuntimeOperations::Neg(RuntimeValue value) {
  if (utils::isNumerical(value)) {
    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(a)>;
          if constexpr (std::is_arithmetic_v<T>) {
            value.setData(a * (-1));
          }
        },
        value.getData());
    return value;
  } else
    throw VM_error(
        "A non-numeric data type cannot be used with negative operator");
}

RuntimeValue RuntimeOperations::Not(const RuntimeValue &value) {
  RuntimeValue newValue;
  if (utils::isNumerical(value)) {
    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(a)>;
          if constexpr (std::is_arithmetic_v<T>) {
            newValue =
                RuntimeValue(Datatype::Int, static_cast<int64_t>(a == 0));
          }
        },
        value.getData());
    return newValue;
  } else
    throw VM_error(
        "A non-numeric data type cannot be used with negative operator");
}
