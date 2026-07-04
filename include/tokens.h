#pragma once
#include <cstdint>
#include <string>
enum class TokenType : uint8_t {
  Identifier,
  Keyword,
  Number,
  Double,
  Symbol,
  Boolean,
  String,
  Operator,
  Separator,
  End,
  Invalid
};

enum class Keyword : uint8_t {
  If,
  Else,

  True,
  False,

  In,
  Out,

  Double,
  Int,
  Char,
  Bool,
  String,

  While,
  For,
  Break,
  Continue,

  Function,
  Return,

  Global,
  Dynamic,
  Const,
  Invalid
};

enum class Separator : uint8_t {
  LeftParenthesis,
  RightParenthesis,
  Colon,
  Semicolon,
  LeftSquareBracket,
  RightSquareBracket,
  LeftCurlyBracket,
  RightCurlyBracket,
  Comma,
  Invalid
};

struct Token {
  TokenType type;
  uint8_t value;
  std::string lexeme;
  size_t lineID;
  size_t columnID;
  Token(TokenType type, const uint8_t &value, std::string lexeme, size_t lineID,
        size_t columnID);
};

inline Token::Token(TokenType type, const uint8_t &value, std::string lexeme,
                    size_t lineID, size_t columnID) {
  this->type = type;
  this->value = value;
  this->lexeme = lexeme;
  this->lineID = lineID;
  this->columnID = columnID;
}
