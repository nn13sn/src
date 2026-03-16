#include "parser.h"
const Token &Parser::peek() const { return tokens[line][pos]; }

Token &Parser::advance() {
  if (isEnd())
    throw std::invalid_argument(
        "Unexepected ending at line: " + std::to_string(peek().lineID) +
        "; column: " + std::to_string(peek().columnID));
  return tokens[line][pos++];
}

Parser::Parser(std::vector<std::vector<Token>> &T) : tokens(T) {}

bool Parser::isEnd() { return peek().type == TokenType::End; }

void Parser::SyntaxErr(const std::string &err) {
  throw std::invalid_argument(err +
                              " at line: " + std::to_string(peek().lineID) +
                              "; column: " + std::to_string(peek().columnID));
}

bool Parser::Check(TokenType type) { return type == peek().type; }

bool Parser::Check(std::string lexeme) { return lexeme == peek().lexeme; }

bool Parser::Check(Keyword keyword) {
  return peek().type == TokenType::Keyword &&
         static_cast<uint8_t>(keyword) == peek().value;
}

bool Parser::Check(Operator op) {
  return peek().type == TokenType::Operator &&
         static_cast<uint8_t>(op) == peek().value;
}

bool Parser::Check(Separator sep) {
  return peek().type == TokenType::Separator &&
         static_cast<uint8_t>(sep) == peek().value;
}

bool Parser::eatEnd() {
  if (Check(TokenType::End)) {
    line++;
    pos = 0;
    return true;
  }
  return false;
}

Datatype Parser::getDatatype(const TokenType &tokentype) {
  if (Check(TokenType::Number))
    return Datatype::Int;
  if (Check(TokenType::Double))
    return Datatype::Double;
  if (Check(TokenType::Symbol))
    return Datatype::Char;
  if (Check(TokenType::Boolean))
    return Datatype::Bool;
  if (Check(TokenType::String))
    return Datatype::String;
  return Datatype::Invalid;
}

Datatype Parser::getDatatype(const Keyword &keyword) {
  switch (keyword) {
  case Keyword::Int:
    return Datatype::Int;
  case Keyword::Double:
    return Datatype::Double;
  case Keyword::Char:
    return Datatype::Char;
  case Keyword::Bool:
    return Datatype::Bool;
  case Keyword::String:
    return Datatype::String;
  default:
    return Datatype::Invalid;
  }
}

std::variant<int64_t, char, std::string, double, bool, std::vector<Value>>
Parser::getData() {
  if (Check(TokenType::Number))
    return std::stoi(peek().lexeme);
  if (Check(TokenType::Double))
    return std::stod(peek().lexeme);
  if (Check(TokenType::Symbol))
    return peek().lexeme[0];
  if (Check(TokenType::String))
    return peek().lexeme;
  if (Check(TokenType::Boolean)) {
    if (peek().lexeme == "true")
      return true;
    if (peek().lexeme == "false")
      return false;
  }
}

std::unique_ptr<Program> Parser::MakeBody() {
  auto body = std::make_unique<Program>();
  eatEnd();
  while (true) {
    if (line >= tokens.size()) {
      line--;
      pos = tokens[line].size() - 1;
      SyntaxErr("Expected \"}\"");
    }
    if (Check(Separator::RightCurlyBracket))
      break;
    body->statements.push_back(MakeStatement());
    if (Check(Separator::RightCurlyBracket))
      break;
    else if (!eatEnd() && pos != 0)
      SyntaxErr("End of the line is expected");
  }
  advance();
  if (isEnd()) {
    if (line == tokens.size() - 1)
      return body;
    line++;
    pos = 0;
  }
  return body;
}

std::unique_ptr<Expression> Parser::SingleParse() {
  if (Check(TokenType::Number) || Check(TokenType::Double) ||
      Check(TokenType::Boolean) || Check(TokenType::Symbol) ||
      Check(TokenType::String)) {
    auto expr = std::make_unique<exprValue>();
    expr->value.type = getDatatype(peek().type);
    expr->value.data = getData();
    expr->location.line = peek().lineID;
    expr->location.column = advance().columnID;
    return expr;
  } else if (Check(TokenType::Identifier)) {
    auto expr = std::make_unique<Variable>();
    expr->name = peek().lexeme;
    expr->location.line = peek().lineID;
    expr->location.column = advance().columnID;
    return expr;
  } else if (Check(TokenType::Keyword)) {
    auto expr = std::make_unique<Cast>();
    expr->castTo = getDatatype(static_cast<Keyword>(peek().value));
    if (expr->castTo == Datatype::Invalid)
      SyntaxErr("A valid data type is expected");
    expr->location.line = peek().lineID;
    expr->location.column = advance().columnID;
    if (Check(Separator::LeftParenthesis))
      advance();
    else
      SyntaxErr(OPENPARENTHESIS);
    expr->expr = MakeExpression();
    if (Check(Separator::RightParenthesis))
      advance();
    else
      SyntaxErr(CLOSEPARENTHESIS);
    return expr;
  } else if (Check(Separator::LeftParenthesis)) {
    advance();
    auto expr = MakeExpression();
    if (Check(Separator::RightParenthesis))
      advance();
    else
      SyntaxErr(CLOSEPARENTHESIS);
    return expr;
  }
  SyntaxErr("Invalid component of the expression");
  return nullptr;
}

std::unique_ptr<Expression> Parser::MakeExpression() {
  auto expr = MakeLogical();
  while (Check(Operator::AND) || Check(Operator::OR)) {
    auto bin = std::make_unique<Binary>();
    auto op = static_cast<Operator>(peek().value);
    bin->location.line = peek().lineID;
    bin->location.column = advance().columnID;
    bin->op = op;
    bin->right = MakeLogical();
    bin->left = std::move(expr);
    expr = std::move(bin);
  }
  return expr;
}

std::unique_ptr<Expression> Parser::MakeLogical() {
  auto expr = ParseMidTerm();

  while (Check(Operator::Greater) || Check(Operator::Less) ||
         Check(Operator::Equal) || Check(Operator::GreaterEq) ||
         Check(Operator::LessEq) || Check(Operator::NotEqual)) {
    auto bin = std::make_unique<Binary>();
    auto op = static_cast<Operator>(peek().value);
    bin->location.line = peek().lineID;
    bin->location.column = advance().columnID;
    bin->op = op;
    bin->right = ParseMidTerm();
    bin->left = std::move(expr);
    expr = std::move(bin);
  }
  return expr;
}

std::unique_ptr<Expression> Parser::ParseTerm() {
  auto expr = SingleParse();

  while (Check(Operator::Mul) || Check(Operator::Div) || Check(Operator::Mod)) {
    auto bin = std::make_unique<Binary>();
    auto op = static_cast<Operator>(peek().value);
    bin->location.line = peek().lineID;
    bin->location.column = advance().columnID;
    bin->op = op;
    bin->right = SingleParse();
    bin->left = std::move(expr);
    expr = std::move(bin);
  }
  return expr;
}

std::unique_ptr<Expression> Parser::ParseMidTerm() {
  auto expr = ParseTerm();

  while (Check(Operator::Add) || Check(Operator::Sub)) {
    auto bin = std::make_unique<Binary>();
    auto op = static_cast<Operator>(peek().value);
    bin->location.line = peek().lineID;
    bin->location.column = advance().columnID;
    bin->op = op;
    bin->right = ParseTerm();
    bin->left = std::move(expr);
    expr = std::move(bin);
  }
  return expr;
}

std::unique_ptr<Statement> Parser::ParseInput() {
  auto stmt = std::make_unique<Input>();
  stmt->location.line = advance().lineID;
  if (Check(Separator::LeftParenthesis))
    advance();
  else
    SyntaxErr(OPENPARENTHESIS);
  stmt->input = MakeExpression();
  if (Check(Separator::RightParenthesis))
    advance();
  else
    SyntaxErr(CLOSEPARENTHESIS);
  return stmt;
}

std::unique_ptr<Statement> Parser::ParseOutput() {
  auto stmt = std::make_unique<Output>();
  stmt->location.line = advance().lineID;
  if (Check(Separator::LeftParenthesis))
    advance();
  else
    SyntaxErr(OPENPARENTHESIS);
  stmt->output = MakeExpression();
  if (Check(Separator::RightParenthesis))
    advance();
  else
    SyntaxErr(CLOSEPARENTHESIS);
  return stmt;
}

std::unique_ptr<Statement> Parser::ParseDefinition() {
  auto stmt = std::make_unique<Definition>();
  stmt->name = advance().lexeme;
  if (Check(Operator::Def))
    stmt->location.line = advance().lineID;
  else
    SyntaxErr("Expected \"=\"");
  stmt->value = MakeExpression();
  return stmt;
}

std::unique_ptr<Statement> Parser::ParseIfStatement() {
  auto stmt = std::make_unique<IfStatement>();
  stmt->location.line = advance().lineID;
  if (Check(Separator::LeftParenthesis))
    advance();
  else
    SyntaxErr(OPENPARENTHESIS);
  stmt->expr = MakeExpression();
  if (Check(Separator::RightParenthesis))
    advance();
  else
    SyntaxErr(CLOSEPARENTHESIS);
  eatEnd();
  if (Check(Separator::LeftCurlyBracket))
    advance();
  else
    SyntaxErr(OPENCURLYBRACKET);
  stmt->Instructions = MakeBody();
  if (Check(Keyword::Else)) {
    stmt->elseStatement = std::make_unique<IfStatement>();
    stmt->location.line = advance().lineID;
    eatEnd();
    if (Check(Separator::LeftCurlyBracket)) {
      advance();
      stmt->elseStatement->expr = nullptr;
      stmt->elseStatement->Instructions = MakeBody();
    } else if (Check(Keyword::If)) {
      stmt->elseStatement.reset(
          static_cast<IfStatement *>(ParseIfStatement().release()));
    } else
      SyntaxErr(OPENCURLYBRACKET);
  }
  return stmt;
}

std::unique_ptr<Statement> Parser::ParseWhile() {
  auto stmt = std::make_unique<While>();
  stmt->location.line = advance().lineID;
  if (Check(Separator::LeftParenthesis))
    advance();
  else
    SyntaxErr(OPENPARENTHESIS);
  stmt->expr = MakeExpression();
  if (Check(Separator::RightParenthesis))
    advance();
  else
    SyntaxErr(CLOSEPARENTHESIS);
  eatEnd();
  if (Check(Separator::LeftCurlyBracket))
    advance();
  else
    SyntaxErr(OPENCURLYBRACKET);
  stmt->Instructions = MakeBody();
  return stmt;
}

std::unique_ptr<Statement> Parser::ParseFor() {
  auto stmt = std::make_unique<For>();
  stmt->location.line = advance().lineID;
  if (Check(Separator::LeftParenthesis))
    advance();
  else
    SyntaxErr(OPENPARENTHESIS);
  stmt->Initialvalue->value = nullptr;
  if (Check(TokenType::Identifier)) {
    stmt->Initialvalue->location.line = peek().lineID;
    stmt->Initialvalue->name = advance().lexeme;
  } else
    SyntaxErr("Variable (iterator) is expected");
  if (Check(Operator::Def)) {
    advance();
    stmt->Initialvalue->value = MakeExpression();
  }
  if (Check(Operator::Arrow) || Check(Operator::ArrowEq) ||
      Check(Operator::NotEqual) || Check(Operator::Greater) ||
      Check(Operator::Less) || Check(Operator::LessEq) ||
      Check(Operator::GreaterEq)) {
    stmt->op = static_cast<Operator>(advance().value);
  } else
    SyntaxErr("Invalid operator or not an operator");
  stmt->Finalvalue = MakeExpression();
  if (Check(Separator::LeftParenthesis)) {
    advance();
    if (Check(TokenType::Identifier))
      stmt->step.reset(static_cast<Definition *>(ParseDefinition().release()));
    if (Check(Separator::RightParenthesis))
      advance();
    else
      SyntaxErr(CLOSEPARENTHESIS);
  }
  if (Check(Separator::RightParenthesis))
    advance();
  else
    SyntaxErr(CLOSEPARENTHESIS);
  eatEnd();
  if (Check(Separator::LeftCurlyBracket))
    advance();
  else
    SyntaxErr(OPENCURLYBRACKET);
  stmt->Instructions = MakeBody();
  return stmt;
}

std::unique_ptr<Statement> Parser::MakeStatement() {
  if (Check(Keyword::Out))
    return ParseOutput();
  else if (Check(Keyword::In))
    return ParseInput();
  else if (Check(TokenType::Identifier))
    return ParseDefinition();
  else if (Check(Keyword::If))
    return ParseIfStatement();
  else if (Check(Keyword::While))
    return ParseWhile();
  else if (Check(Keyword::For))
    return ParseFor();
  SyntaxErr("Cannot match the Syntax");
  return nullptr;
}

void Parser::Parse(Program &program) {
  while (line < tokens.size()) {
    program.statements.push_back(MakeStatement());
    if (!eatEnd() && pos != 0)
      SyntaxErr("End of the line is expected");
  }
}
