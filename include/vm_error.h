#pragma once
#include "location.h"
#include <stdexcept>

class VM_error : public std::runtime_error {
public:
  Location location;
  inline VM_error(const std::string &msg) : std::runtime_error(msg) {};
};
