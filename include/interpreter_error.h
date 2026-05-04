#pragma once
#include "location.h"
#include <stdexcept>
#include <string>
class interpreter_error : public std::runtime_error {
public:
  Location location;
  interpreter_error(const std::string &msg, const Location &loc);
};

inline interpreter_error::interpreter_error(const std::string &msg,
                                            const Location &loc)
    : std::runtime_error(msg) {
  location = loc;
}
