#pragma once

#include <cstdint>
#include <vector>

using VariableID = uint32_t;
using Slot = uint32_t;
struct Slot_Table {
  uint32_t nextID = 0;
  uint32_t nextSlot = 0;
  std::vector<VariableID> IDs = {};
  std::vector<Slot> slots = {};
};
