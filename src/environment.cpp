#include "environment.h"
#include "runtime_value.h"
RuntimeVariable Environment::get(const std::string &name) {
  if (auto value = globals.find(name); value != globals.end())
    return value->second;
  if (auto value = values.find(name); value != values.end()) {
    return value->second;
  }
  if (parent)
    return parent->get(name);
  return RuntimeValue(Datatype::Invalid, NULL);
}

RuntimeVariable *Environment::getPointer(const std::string &name) {
  if (auto value = globals.find(name); value != globals.end())
    return &value->second;
  if (auto value = values.find(name); value != values.end()) {
    return &value->second;
  }
  if (parent)
    return parent->getPointer(name);
  return nullptr;
}

void Environment::set(const std::string &name, const RuntimeValue &value,
                      const Location &loc, const int32_t &mods) {
  if (auto ptr = getPointer(name)) {
    if (mods != 0)
      throw interpreter_error(
          "Modifiers can be used only in the first declaration", loc);
    ptr->set(value, loc);
    return;
  }

  RuntimeVariable var{value, utils::isConst(mods)};

  if (utils::isGlobal(mods))
    globals[name] = var;
  else
    values[name] = var;
}

bool Environment::newGlobal(const std::string &name,
                            const RuntimeValue &value) {
  if (!getPointer(name)) {
    globals[name] = value;
    return true;
  }
  return false;
}
