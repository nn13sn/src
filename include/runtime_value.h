#pragma once
#include "AST.h"
#include <iostream>
#include <string>
struct Function;

using Data = std::variant<int64_t, unsigned char, std::string, double, bool,
                          std::vector<Value>, std::shared_ptr<Function>>;

struct RuntimeValue {
  Datatype type;
  Data data;
  RuntimeValue(const Value &value);
  RuntimeValue(const Datatype &type, const Data &data);
  RuntimeValue();
};

inline RuntimeValue::RuntimeValue(const Value &value) {
  type = value.type;
  std::visit([this](const auto &data) { this->data = data; }, value.data);
}

inline RuntimeValue::RuntimeValue(const Datatype &type, const Data &data) {
  this->type = type;
  this->data = data;
}

inline RuntimeValue::RuntimeValue() {
  type = Datatype::Invalid;
  data = NULL;
};
