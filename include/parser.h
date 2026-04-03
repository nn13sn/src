#pragma once
#include "AST.h"
#include "lexer.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#define OPENPARENTHESIS "Expected \"(\""
#define CLOSEPARENTHESIS "Expected \")\""
#define OPENCURLYBRACKET "Expected \"{\""

class Parser {
private:
  size_t line = 0;
  size_t pos = 0;
  void SyntaxErr(const std::string &err);
  const Token &peek() const;
  const Token &advance();
  const Token &peekNext();
  std::vector<std::vector<Token>> &tokens;
  std::unique_ptr<Statement> MakeStatement();
  std::unique_ptr<Statement> ParseInput();
  std::unique_ptr<Statement> ParseOutput();
  std::unique_ptr<Statement> ParseExpression();
  std::unique_ptr<Statement> ParseIfStatement();
  std::unique_ptr<Statement> ParseWhile();
  std::unique_ptr<Statement> ParseFor();
  std::unique_ptr<Statement> ParseFunction();
  std::unique_ptr<Statement> ParseReturn();
  std::unique_ptr<Expression> MakeExpression();
  std::unique_ptr<Expression> OrParse();
  std::unique_ptr<Expression> AndParse();
  std::unique_ptr<Expression> UnaryParse();
  std::unique_ptr<Expression> ParseTerm();
  std::unique_ptr<Expression> ParseMidTerm();
  std::unique_ptr<Expression> LogicalParse();
  template <typename LowFunc>
  std::unique_ptr<Expression> ParseBinary(LowFunc ParseLower,
                                          const std::vector<Operator> &ops);
  std::unique_ptr<Expression> SingleParse();
  std::unique_ptr<Program> MakeBody();
  bool Check(TokenType type);
  bool Check(std::string lexeme);
  bool Check(Keyword keyword);
  bool Check(Operator op);
  bool Check(Separator sep);
  bool isEnd();
  bool eatEnd();
  Datatype getDatatype();
  Datatype getDatatype(const Keyword &keyword);
  std::variant<int64_t, unsigned char, std::string, double, bool,
               std::vector<Value>>
  getData();

public:
  Parser(std::vector<std::vector<Token>> &T);
  void Parse(Program &program);
};
