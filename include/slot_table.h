#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
struct Slot_Table {
  std::unordered_map<std::string, uint32_t> slots;
  uint32_t nextslot = 0;
};
