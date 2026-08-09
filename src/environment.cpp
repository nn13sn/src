#include "environment.h"
#include "runtime_value.h"
#include "vm_error.h"

Environment::Environment(const uint32_t &size) { locals.resize(size); }

RuntimeVariable Environment::get(const uint32_t &index) {
  if (index < locals.size())
    return locals[index];
  if (parent)
    return parent->get(index);
  return RuntimeValue(Datatype::Invalid, NULL);
}

RuntimeVariable *Environment::getPointer(const uint32_t &index) {
  if (index < locals.size())
    return &locals[index];
  if (parent)
    return parent->getPointer(index);
  return nullptr;
}

void Environment::set(const uint32_t &index, const RuntimeValue &value,
                      const int32_t &mods) {
  if (auto ptr = getPointer(index)) {
    if (mods != 0)
      throw VM_error("Modifiers can be used only with the first declaration");
    ptr->set(value);
    return;
  }

  RuntimeVariable var{value, utils::isConst(mods)};

  locals[index] = var;
}

bool Environment::newGlobal(const uint32_t &index, const RuntimeValue &value) {
  if (!getPointer(index)) {
    globals[index] = value;
    return true;
  }
  return false;
}
