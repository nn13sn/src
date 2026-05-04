#include "runtime_value.h"

RuntimeValue::RuntimeValue(const Literal &value) {
  type = value.type;
  std::visit([this](const auto &data) { this->data = data; }, value.data);
}

RuntimeValue::RuntimeValue(const Datatype &type, const Data &data) {
  this->type = type;
  this->data = data;
}

RuntimeValue::RuntimeValue() {
  type = Datatype::Invalid;
  data = NULL;
}

Datatype RuntimeValue::deduceType(const Data &data) {
  return std::visit(
      [](auto &&a) -> Datatype {
        using T = std::decay_t<decltype(a)>;

        if constexpr (std::is_same_v<T, int64_t>)
          return Datatype::Int;
        else if constexpr (std::is_same_v<T, unsigned char>)
          return Datatype::Char;
        else if constexpr (std::is_same_v<T, double>)
          return Datatype::Double;
        else if constexpr (std::is_same_v<T, bool>)
          return Datatype::Bool;
        else if constexpr (std::is_same_v<T, std::string>)
          return Datatype::String;
        else if constexpr (std::is_same_v<T, std::vector<Literal>>)
          return Datatype::Array;
        else if constexpr (std::is_same_v<T, std::shared_ptr<Function>>)
          return Datatype::Function;
        else
          return Datatype::Invalid;
      },
      data);
}

void RuntimeValue::increment(const Location &loc) {
  if (utils::isNumerical(*this)) {
    std::visit(
        [](auto &c) {
          using T = std::decay_t<decltype(c)>;
          if constexpr (std::is_arithmetic_v<T>) {
            c = c + 1;
          }
        },
        this->data);
  } else
    throw interpreter_error(
        "The increment operator cannot be used to such value type", loc);
}

void RuntimeValue::decrement(const Location &loc) {
  if (utils::isNumerical(*this)) {
    std::visit(
        [](auto &c) {
          using T = std::decay_t<decltype(c)>;
          if constexpr (std::is_arithmetic_v<T>) {
            c = c - 1;
          }
        },
        this->data);
  } else
    throw interpreter_error(
        "The increment operator cannot be used to such value type", loc);
}

void RuntimeValue::set(const RuntimeValue &var, const Location &loc) {
  *this = var;
}

void RuntimeValue::setData(const Data &data, const Location &loc) {
  this->data = data;
  type = deduceType(data);
}

const Data &RuntimeValue::getData() const { return this->data; }

const Datatype &RuntimeValue::getType() const { return this->type; }
