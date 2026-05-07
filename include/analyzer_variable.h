#pragma once
#include <cstdint>
struct AnalyzerVariable {
  bool isConst = false;
  int32_t locked = 0;
  inline void lock() { locked++; }
  inline void unlock() { locked--; }
  inline bool isallowed() { return !(isConst || locked); }
  AnalyzerVariable() {};
};
