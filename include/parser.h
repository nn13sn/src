#pragma once
#include "AST.h"
#include "lexer.h"
#include "parser_error.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#define OPENPARENTHESIS "Expected \"(\""
#define CLOSEPARENTHESIS "Expected \")\""
#define OPENCURLYBRACKET "Expected \"{\""
#define PARSER_OK 1
#define PARSER_ERROR -1

class Parser {
private:
  size_t line = 0;
  size_t pos = 0;
  void SyntaxErr(const std::string &err);
  const Token &peek() const; // get the current tokens
  const Token &advance();    // move to the next token & get the current one
  const Token &peekNext();   // get the next token
  bool getToBracket();
  std::vector<std::vector<Token>> &tokens;
  std::vector<ParserError> errors = {};
  std::unique_ptr<Statement> MakeStatement(); // creating new statement
  // Parsing all possible statement types
  std::unique_ptr<Statement> ParseInput();
  std::unique_ptr<Statement> ParseOutput();
  std::unique_ptr<Statement> ParseExpression();
  std::unique_ptr<Statement> ParseBlock();
  std::unique_ptr<Statement> ParseIfStatement();
  std::unique_ptr<Statement> ParseWhile();
  std::unique_ptr<Statement> ParseFor();
  std::unique_ptr<Statement> ParseBreak();
  std::unique_ptr<Statement> ParseContinue();
  std::unique_ptr<Statement> ParseFunction();
  std::unique_ptr<Statement> ParseReturn();
  std::unique_ptr<Statement> ParseModifiers();
  // Parsing expressions
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

  std::unique_ptr<Program> MakeBody(); // creating blocks
  bool Check(TokenType type);
  bool Check(std::string lexeme);
  bool Check(Keyword keyword);
  bool Check(Operator op);
  bool Check(Separator sep);
  bool isEnd();
  bool eatEnd();
  Datatype getDatatype();
  Datatype getDatatype(const Keyword &keyword);
  std::variant<dc_int, dc_double, dc_char, dc_bool, dc_string,
               std::vector<Literal>>
  getData();
  void addModifier(int32_t &mods, const Keyword &keywrd);

public:
  Parser(std::vector<std::vector<Token>> &T);
  signed char Parse(Program &program);
  void printErrors();
};
