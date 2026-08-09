#pragma once
#include "tokens.h"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
class Lexer {
  static constexpr std::array<std::string_view,
                              static_cast<size_t>(Keyword::Invalid)>
      keywords{"if",       "else",   "true",   "false",   "in",
               "out",      "double", "int",    "char",    "bool",
               "string",   "while",  "for",    "break",   "continue",
               "function", "return", "global", "dynamic", "const"};
  uint8_t IsKeyword(const std::string_view lexeme);
  std::vector<std::string> Initialcode;
  std::vector<std::vector<Token>> tokens;
  size_t i;
  size_t pos;

  bool isLetter();
  bool isDigit();
  bool isText();
  bool isOperator();
  bool isSeparator();
  char getEscapes(const char &c);
  void unexEnd();
  std::vector<std::string> errors = {};

public:
  std::vector<std::vector<Token>> Tokenize();
  void readFile(std::string name);
};
