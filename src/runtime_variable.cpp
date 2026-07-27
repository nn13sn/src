#include "runtime_variable.h"
#include "runtime_value.h"
#include "vm_error.h"
bool RuntimeVariable::CheckConst() { return !(isConst || locked); }

void RuntimeVariable::lock() { locked++; }

void RuntimeVariable::unlock() { locked--; }

RuntimeValue &RuntimeVariable::get() { return value; }

void RuntimeVariable::increment() {
  if (isConst)
    throw VM_error("Cannot modify constant");
  if (locked)
    throw VM_error("Cannot modify loop iterator");
  value.increment();
}

void RuntimeVariable::decrement() {
  if (isConst)
    throw VM_error("Cannot modify constant");
  if (locked)
    throw VM_error("Cannot modify loop iterator");
  value.decrement();
}

void RuntimeVariable::set(const RuntimeValue &value) {
  if (isConst)
    throw VM_error("Cannot modify constant");
  if (locked)
    throw VM_error("Cannot modify loop iterator");
  this->value = value;
}

void RuntimeVariable::setData(const Data &data) {
  if (isConst)
    throw VM_error("Cannot modify constant");
  if (locked)
    throw VM_error("Cannot modify loop iterator");
  value.setData(data);
}
