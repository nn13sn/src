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
  void increment();
  void decrement();
  void set(const RuntimeValue &value);
  void setData(const Data &data);
  RuntimeVariable(const RuntimeValue &val, const bool &isconst = false)
      : value(val), isConst(isconst) {};
  RuntimeVariable() {};
};
