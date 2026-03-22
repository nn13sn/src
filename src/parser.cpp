#include "parser.h"
#include "lexer.h"
#include <memory>
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

template <typename LowFunc> // I made this method to avoid the code repetion
                            // when parsing each level
std::unique_ptr<Expression>
Parser::ParseBinary(LowFunc ParseLower, const std::vector<Operator> &ops) {
  auto expr = ParseLower();
  while (Check(TokenType::Operator) &&
         std::find(ops.begin(), ops.end(),
                   static_cast<Operator>(peek().value)) != ops.end()) {
    auto bin = std::make_unique<Binary>();
    bin->op = static_cast<Operator>(peek().value);
    bin->location.line = peek().lineID;
    bin->location.column = advance().columnID;
    bin->right = ParseLower();
    bin->left = std::move(expr);
    expr = std::move(bin);
  }
  return expr;
}

std::unique_ptr<Expression> Parser::MakeExpression() {
  auto expr = OrParse();
  if (Check(Operator::Def)) {
    auto def = std::make_unique<Binary>();
    def->op = Operator::Def;
    def->location.line = peek().lineID;
    def->location.column = advance().columnID;
    def->right = MakeExpression();
    def->left = std::move(expr);
    return def;
  }
  return expr;
}

std::unique_ptr<Expression> Parser::OrParse() {
  return ParseBinary([this]() { return AndParse(); }, {Operator::OR});
}

std::unique_ptr<Expression> Parser::AndParse() {
  return ParseBinary([this]() { return LogicalParse(); }, {Operator::AND});
}

std::unique_ptr<Expression> Parser::LogicalParse() {
  return ParseBinary([this]() { return ParseMidTerm(); },
                     {Operator::Greater, Operator::Less, Operator::GreaterEq,
                      Operator::LessEq, Operator::Equal, Operator::NotEqual});
}

std::unique_ptr<Expression> Parser::ParseMidTerm() {
  return ParseBinary([this]() { return ParseTerm(); },
                     {Operator::Add, Operator::Sub});
}

std::unique_ptr<Expression> Parser::ParseTerm() {
  return ParseBinary([this]() { return UnaryParse(); },
                     {Operator::Mul, Operator::Div, Operator::Mod});
}

std::unique_ptr<Expression> Parser::UnaryParse() {
  if (Check(Operator::PreIncr) || Check(Operator::PreDecr) ||
      Check(Operator::Not) || Check(Operator::Sub)) {
    auto unary = std::make_unique<Unary>();
    unary->op = static_cast<Operator>(peek().value);
    unary->location.line = peek().lineID;
    unary->location.column = advance().columnID;
    unary->expr = UnaryParse();
    return unary;
  }
  auto expr = SingleParse();
  while (Check(Operator::PreIncr) || Check(Operator::PreDecr)) {
    auto unary = std::make_unique<Unary>();
    unary->op =
        Check(Operator::PreIncr) ? Operator::PostIncr : Operator::PostDecr;
    unary->location.line = peek().lineID;
    unary->location.column = advance().columnID;
    unary->expr = std::move(expr);
    expr = std::move(unary);
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

std::unique_ptr<Statement> Parser::ParseExpression() {
  auto stmt = std::make_unique<ExpressionStmt>();
  stmt->location.line = peek().lineID;
  stmt->expr = MakeExpression();
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
  if (Check(TokenType::Identifier)) {
    stmt->iterator = advance().lexeme;
  } else
    SyntaxErr("Variable (iterator) is expected");
  if (Check(Operator::Def)) {
    advance();
    stmt->Initialvalue = ParseMidTerm();
  } else
    stmt->Initialvalue = nullptr;
  if (Check(Operator::Arrow) || Check(Operator::ArrowEq) ||
      Check(Operator::NotEqual) || Check(Operator::Greater) ||
      Check(Operator::Less) || Check(Operator::GreaterEq) ||
      Check(Operator::LessEq)) {
    stmt->op = static_cast<Operator>(advance().value);
    stmt->Finalvalue = MakeExpression();
  } else
    SyntaxErr("A correct operator is expected");
  if (Check(Separator::Colon)) {
    advance();
    stmt->step = std::make_unique<Expression>();
    stmt->step = MakeExpression();
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
  else if (Check(TokenType::Identifier) || Check(TokenType::Operator) ||
           Check(TokenType::Separator) || Check(TokenType::String) ||
           Check(TokenType::Boolean) || Check(TokenType::Double) ||
           Check(TokenType::Symbol) || Check(TokenType::Number))
    return ParseExpression();
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
