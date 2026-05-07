#include "runtime_variable.h"
#include "runtime_value.h"

bool RuntimeVariable::CheckConst() { return !(isConst || locked); }

void RuntimeVariable::lock() { locked++; }

void RuntimeVariable::unlock() { locked--; }

RuntimeValue &RuntimeVariable::get() { return value; }

void RuntimeVariable::increment(const Location &loc) {
  if (isConst)
    throw interpreter_error("Cannot modify constant", loc);
  if (locked)
    throw interpreter_error("Cannot modify loop iterator", loc);
  value.increment(loc);
}

void RuntimeVariable::decrement(const Location &loc) {
  if (isConst)
    throw interpreter_error("Cannot modify constant", loc);
  if (locked)
    throw interpreter_error("Cannot modify loop iterator", loc);
  value.decrement(loc);
}

void RuntimeVariable::set(const RuntimeValue &value, const Location &loc) {
  if (isConst)
    throw interpreter_error("Cannot modify constant", loc);
  if (locked)
    throw interpreter_error("Cannot modify loop iterator", loc);
  this->value = value;
}

void RuntimeVariable::setData(const Data &data, const Location &loc) {
  if (isConst)
    throw interpreter_error("Cannot modify constant", loc);
  if (locked)
    throw interpreter_error("Cannot modify loop iterator", loc);
  value.setData(data, loc);
}
