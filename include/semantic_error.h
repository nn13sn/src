#pragma once
#include "AST.h"
#include <string>
struct SemanticError {
  Location loc;
  std::string err;
  SemanticError(const std::string &err, const Location &loc);
};

inline SemanticError::SemanticError(const std::string &err,
                                    const Location &loc) {
  this->err = err;
  this->loc = loc;
}
