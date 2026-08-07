#include "runtime_streams.h"
#include "vm_error.h"
void RuntimeStreams::Print(const RuntimeValue &value) {
  std::visit([&](const auto &a) { PrintValue(a); }, value.getData());
}

void RuntimeStreams::PrintValue(const dc_int &value) { std::cout << value; }

void RuntimeStreams::PrintValue(const dc_double &value) { std::cout << value; }

void RuntimeStreams::PrintValue(const dc_char &value) {
  std::string result;
  if (value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF))
    result = "\xEF\xBF\xBD";
  if (value <= 0x7F) {
    result += static_cast<char>(value);
  } else if (value <= 0x7FF) {
    result += static_cast<char>(0xC0 | ((value >> 6) & 0x1F));
    result += static_cast<char>(0x80 | (value & 0x3F));
  } else if (value <= 0xFFFF) {
    result += static_cast<char>(0xE0 | ((value >> 12) & 0x0F));
    result += static_cast<char>(0x80 | ((value >> 6) & 0x3F));
    result += static_cast<char>(0x80 | (value & 0x3F));
  } else if (value <= 0x10FFFF) {
    result += static_cast<char>(0xF0 | ((value >> 18) & 0x07));
    result += static_cast<char>(0x80 | ((value >> 12) & 0x3F));
    result += static_cast<char>(0x80 | ((value >> 6) & 0x3F));
    result += static_cast<char>(0x80 | (value & 0x3F));
  } // converting char32_t to string because std::cout does not support
    // char32_t. I did not personaly code this one, just took it from ai
  std::cout << result;
}

void RuntimeStreams::PrintValue(const dc_bool &value) {
  std::cout << (value ? "true" : "false");
}

void RuntimeStreams::PrintValue(const dc_string &value) { std::cout << value; }

void RuntimeStreams::PrintValue(const auto &) {
  throw VM_error("Such data type cannot be printed");
}
