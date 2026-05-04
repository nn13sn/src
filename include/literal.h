#pragma once
#include "datatype.h"
#include <string>
#include <variant>
#include <vector>
struct Literal {
  Datatype type;
  std::variant<int64_t, unsigned char, std::string, double, bool,
               std::vector<Literal>>
      data;
};
