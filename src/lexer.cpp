#include "lexer.h"
#include <string>

void Lexer::readFile(std::string name) {
  std::string line;
  std::ifstream in(name);
  if (in.is_open()) {
    while (std::getline(in, line)) {
      Initialcode.push_back(line);
    }
  } else {
    std::cout << "Cannot open/find such file\n";
    std::exit(1);
  }
}

Token::Token(TokenType type, const uint8_t &value, std::string lexeme,
             size_t lineID, size_t columnID) {
  this->type = type;
  this->value = value;
  this->lexeme = lexeme;
  this->lineID = lineID;
  this->columnID = columnID;
}

void Lexer::unexEnd() {
  if (pos == Initialcode[i].size())
    throw std::invalid_argument(
        "Unexpected ending, at line: " + std::to_string(i) +
        "; column: " + std::to_string(pos));
}

uint8_t Lexer::IsKeyword(const std::string_view lexeme) {
  for (size_t i = 0; i < keywords.size(); i++) {
    if (lexeme == keywords[i])
      return static_cast<uint8_t>(i);
  }
  return UINT8_MAX;
}

char Lexer::getEscapes(const char &c) {
  switch (c) {
  case 'n':
    return '\n';
  case '"':
    return '\"';
  case '\\':
    return '\\';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case '\'':
    return '\'';
  case '0':
    return '\0';
  default:
    return -1;
  }
}

bool Lexer::isLetter() {
  if (std::isalpha(Initialcode[i][pos]) || Initialcode[i][pos] == '_') {
    size_t startpos = pos;
    while (pos < Initialcode[i].size() &&
           (std::isalpha(Initialcode[i][pos]) || Initialcode[i][pos] == '_')) {
      pos++;
    }
    pos--;
    auto lexeme = std::string_view(Initialcode[i].data() + startpos,
                                   Initialcode[i].data() + pos + 1);
    if (lexeme == "true" || lexeme == "false") {
      tokens.back().emplace_back(TokenType::Boolean, UINT8_MAX,
                                 std::string(lexeme), i + 1,
                                 pos - lexeme.size() + 2);
      return true;
    }
    auto result = IsKeyword(lexeme);
    if (result != UINT8_MAX)
      tokens.back().emplace_back(TokenType::Keyword, result,
                                 std::string(lexeme), i + 1,
                                 pos - lexeme.size() + 2);
    else
      tokens.back().emplace_back(TokenType::Identifier, result,
                                 std::string(lexeme), i + 1,
                                 pos - lexeme.size() + 2);
    return true;
  }
  return false;
}

bool Lexer::isDigit() {
  size_t startpos = pos;
  if (std::isdigit(Initialcode[i][pos])) {
    while (pos < Initialcode[i].size() && (std::isdigit(Initialcode[i][pos]))) {
      pos++;
    }
    if (pos < Initialcode[i].size() && Initialcode[i][pos] == '.') {
      pos++;
      while (pos < Initialcode[i].size() && std::isdigit(Initialcode[i][pos])) {
        pos++;
      }
      tokens.back().emplace_back(TokenType::Double, UINT8_MAX,
                                 std::string(Initialcode[i].data() + startpos,
                                             Initialcode[i].data() + pos),
                                 i + 1, startpos + 1);
      pos--;
      return true;
    }
    tokens.back().emplace_back(TokenType::Number, UINT8_MAX,
                               std::string(Initialcode[i].data() + startpos,
                                           Initialcode[i].data() + pos),
                               i + 1, startpos + 1);
    pos--;
    return true;
  }
  return false;
}

bool Lexer::isText() {
  if (Initialcode[i][pos] == '\'') {
    auto startpos = pos;
    pos++;
    if (Initialcode[i][pos] == '\\') {
      pos++;
      char c = getEscapes(Initialcode[i][pos]);
      if (c == -1)
        throw std::invalid_argument(
            "Invalid Escape Sequence at line: " + std::to_string(i) +
            "; column: " + std::to_string(pos));
      tokens.back().emplace_back(TokenType::Symbol, UINT8_MAX,
                                 std::string(1, c), i + 1, startpos + 1);
    } else {
      tokens.back().emplace_back(TokenType::Symbol, UINT8_MAX,
                                 std::string(1, Initialcode[i][pos]), i + 1,
                                 startpos + 1);
    }
    if (Initialcode[i][++pos] != '\'')
      throw std::invalid_argument(
          "Invalid argument for char, at line: " + std::to_string(i) +
          "; column: " + std::to_string(pos));
    return true;
  } else if (Initialcode[i][pos] == '"') {
    std::string str = "";
    auto startpos = pos;
    pos++;
    while (Initialcode[i][pos] != '"') {
      if (Initialcode[i][pos] == '\\')
        str += getEscapes(Initialcode[i][++pos]);
      else
        str += Initialcode[i][pos];
      pos++;
      unexEnd();
    }
    tokens.back().emplace_back(TokenType::String, UINT8_MAX, str, i + 1,
                               startpos + 1);
    return true;
  }
  return false;
}

bool Lexer::isOperator() {
  auto startpos = pos;
  Operator op = Operator::Invalid; // I could use numbers (uint8_t) right away
                                   // but for safety i decided not to do so
  switch (Initialcode[i][pos]) {
  case '+':
    op = Operator::Add;
    break;
  case '*':
    op = Operator::Mul;
    break;
  case '/':
    op = Operator::Div;
    break;
  case '%':
    op = Operator::Mod;
    break;
  case '-':
    if (Initialcode[i][pos + 1] == '>') {
      ++pos;
      if (Initialcode[i][pos + 1] == '=') {
        ++pos;
        op = Operator::ArrowEq;
      } else
        op = Operator::Arrow;
    } else
      op = Operator::Sub;
    break;
  case '>':
    if (Initialcode[i][pos + 1] == '=') {
      ++pos;
      op = Operator::GreaterEq;
    } else
      op = Operator::Greater;
    break;
  case '<':
    if (Initialcode[i][pos + 1] == '=') {
      ++pos;
      op = Operator::LessEq;
    } else
      op = Operator::Less;
    break;
  case '=':
    if (Initialcode[i][pos + 1] == '=') {
      ++pos;
      op = Operator::Equal;
    } else
      op = Operator::Def;
    break;
  case '!':
    if (Initialcode[i][pos + 1] == '=') {
      ++pos;
      op = Operator::NotEqual;
    } else
      op = Operator::Not;
    break;
  case '&':
    if (Initialcode[i][pos + 1] == '&') {
      ++pos;
      op = Operator::AND;
    } else
      throw std::invalid_argument(
          "Invalid operator at line: " + std::to_string(i) +
          "; column: " + std::to_string(pos));
    break;
  case '|':
    if (Initialcode[i][pos + 1] == '|') {
      ++pos;
      op = Operator::OR;
    } else
      throw std::invalid_argument(
          "Invalid operator at line: " + std::to_string(i) +
          "; column: " + std::to_string(pos));
    break;
  }
  if (op != Operator::Invalid) {
    tokens.back().emplace_back(TokenType::Operator, static_cast<uint8_t>(op),
                               "", i + 1, startpos + 1);
    return true;
  }
  return false;
}

bool Lexer::isSeparator() {
  Separator sep = Separator::Invalid;
  switch (Initialcode[i][pos]) {
  case '(':
    sep = Separator::LeftParenthesis;
    break;
  case ')':
    sep = Separator::RightParenthesis;
    break;
  case ':':
    sep = Separator::Colon;
    break;
  case ';':
    sep = Separator::Semicolon;
    break;
  case '[':
    sep = Separator::LeftSquareBracket;
    break;
  case ']':
    sep = Separator::RightSquareBracket;
    break;
  case '{':
    sep = Separator::LeftCurlyBracket;
    break;
  case '}':
    sep = Separator::RightCurlyBracket;
    break;
  }
  if (sep != Separator::Invalid) {
    tokens.back().emplace_back(TokenType::Separator, static_cast<uint8_t>(sep),
                               "", i + 1, pos + 1);
    return true;
  }
  return false;
}

std::vector<std::vector<Token>> Lexer::Tokenize() {
  for (i = 0; i < Initialcode.size(); i++) {
    if (Initialcode[i].size() == 0)
      continue;
    tokens.emplace_back();
    for (pos = 0; pos < Initialcode[i].size(); pos++) {
      if (std::isspace(Initialcode[i][pos]))
        continue;
      else if (isLetter())
        continue;
      else if (isDigit())
        continue;
      else if (isText())
        continue;
      else if (isOperator())
        continue;
      else if (isSeparator())
        continue;
      else
        throw std::invalid_argument("Invalid symbol at line " +
                                    std::to_string(i) +
                                    "; column: " + std::to_string(pos));
    }
    tokens.back().emplace_back(TokenType::End, UINT8_MAX, "", i + 1,
                               Initialcode[i].size() + 1);
  }
  return tokens;
}
