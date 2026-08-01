#pragma once
#include "interpreter_error.h"
#include "literal.h"
#include "utils.h"
#include <string>
#include <variant>
#include <vector>
struct Function {
  const FunctionStatement *declaration;
};

using Data = std::variant<dc_int, dc_double, dc_char, dc_bool, dc_string,
                          std::vector<Literal>, std::shared_ptr<Function>>;

class RuntimeValue {
  Datatype type;
  Data data;
  Datatype deduceType(const Data &data);

public:
  RuntimeValue(const Literal &value);
  RuntimeValue(const Datatype &type, const Data &data);
  RuntimeValue();
  void increment();
  void decrement();
  void set(const RuntimeValue &var);
  void setData(const Data &data);
  const Data &getData() const;
  const Datatype &getType() const;
};
