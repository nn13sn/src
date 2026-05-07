#pragma once
#include "runtime_value.h"

class RuntimeVariable {
  RuntimeValue value;
  bool isConst = false;
  int32_t locked = 0;

public:
  bool CheckConst();
  void lock();
  void unlock();
  RuntimeValue &get();
  void increment(const Location &loc);
  void decrement(const Location &loc);
  void set(const RuntimeValue &value, const Location &loc);
  void setData(const Data &data, const Location &loc);
  RuntimeVariable(const RuntimeValue &val, const bool &isconst = false)
      : value(val), isConst(isconst) {};
  RuntimeVariable() {};
};
