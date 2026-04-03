#include "interpreter.h"
#include "AST.h"
#include <memory>

bool insidefunction = false;

interpreter_error::interpreter_error(const std::string &msg,
                                     const Location &loc)
    : std::runtime_error(msg) {
  location = loc;
}

RuntimeValue::RuntimeValue(const Value &value) {
  type = value.type;
  std::visit([this](const auto &data) { this->data = data; }, value.data);
}

RuntimeValue::RuntimeValue(const Datatype &type, const Data &data) {
  this->type = type;
  this->data = data;
}

RuntimeValue::RuntimeValue() {
  type = Datatype::Invalid;
  data = NULL;
};
// I need a default constructor because when std::unordered_map creates an
// empty object first and only then changes it

RuntimeValue Environment::get(const std::string &name) {
  if (auto value = values.find(name); value != values.end()) {
    return value->second;
  }
  if (parent)
    return parent->get(name);
  return {Datatype::Invalid, NULL};
}

RuntimeValue *Environment::getPointer(const std::string &name) {
  if (auto value = values.find(name); value != values.end()) {
    return &value->second;
  }
  if (parent)
    return parent->getPointer(name);
  return nullptr;
}

void Environment::set(const std::string &name, const RuntimeValue &value) {
  if (auto ptr = getPointer(name)) {
    *ptr = value;
    return;
  }
  values[name] = value;
}

void Interpreter::addScope() {
  environment = std::make_shared<Environment>(Environment{{}, environment});
}

void Interpreter::popScope() {
  if (environment->parent) {
    environment = environment->parent;
  }
  // i could add an exeption here but it should probably be detected at the
  // parser level
}

RuntimeValue Interpreter::validCheck(const RuntimeValue &value,
                                     const Location &loc,
                                     const std::string &name) {
  if (value.type == Datatype::Invalid) {
    throw interpreter_error("Undefined variable: " + name, loc);
  }
  return value;
}

RuntimeValue *Interpreter::validCheck(RuntimeValue *ptr, const Location &loc,
                                      const std::string &name) {
  if (!ptr) {
    throw interpreter_error("Undefined variable: " + name, loc);
  }
  return ptr;
}

RuntimeValue Interpreter::eval(const Expression &expr) {
  if (auto a = dynamic_cast<const exprValue *>(&expr))
    return a->value;
  else if (auto a = dynamic_cast<const Variable *>(&expr)) {
    return validCheck(environment->get(a->name), a->location, a->name);
  } else if (auto a = dynamic_cast<const FunctionCall *>(&expr)) {
    return evalFunctionCall(*a);
  } else if (auto a = dynamic_cast<const Binary *>(&expr)) {
    switch (a->op) {
    case Operator::Add:
      return evalAdd(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Sub:
      return evalSub(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Mul:
      return evalMul(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Div:
      return evalDiv(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Mod:
      return evalMod(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Greater:
      return evalGr(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Less:
      return evalLs(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Equal:
      return evalEq(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::NotEqual:
      return evalNq(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::GreaterEq:
      return evalGe(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::LessEq:
      return evalLe(eval(*(a->left)), eval(*(a->right)), a->location);
    case Operator::Def:
      return evalDef(*a);
    case Operator::AND:
      if (!isTrue(eval(*(a->left)), a->location))
        return {Datatype::Bool, false};
      if (!isTrue(eval(*(a->right)), a->location))
        return {Datatype::Bool, false};
      return {Datatype::Bool, true};
    case Operator::OR:
      if (isTrue(eval(*(a->left)), a->location))
        return {Datatype::Bool, true};
      if (isTrue(eval(*(a->right)), a->location))
        return {Datatype::Bool, true};
      return {Datatype::Bool, false};
    default:
      throw interpreter_error("A binary operator is expected", a->location);
    }
  } else if (auto a = dynamic_cast<const Cast *>(&expr)) {
    auto b = eval(*a->expr);
    if (b.type == Datatype::String)
      return convertString(*a);
    switch (a->castTo) {
    case Datatype::Int:
      return {Datatype::Int, toInt(b, a->location)};
    case Datatype::Double:
      return {Datatype::Double, toDouble(b, a->location)};
    case Datatype::Char:
      return {Datatype::Char, toChar(b, a->location)};
    case Datatype::Bool:
      return {Datatype::Bool, isTrue(b, a->location)};
    case Datatype::String:
      return {Datatype::String, toString(b, a->location)};
    default:
      throw interpreter_error("Invalid data type to be casted to", a->location);
    }
  } else if (auto a = dynamic_cast<const Unary *>(&expr)) {
    switch (a->op) {
    case Operator::Not:
      return {Datatype::Bool, !isTrue(eval(*a->expr), a->location)};
    case Operator::PreIncr:
      return evalPreIncr(*a);
    case Operator::PreDecr:
      return evalPreDecr(*a);
    case Operator::PostIncr:
      return evalPostIncr(*a);
    case Operator::PostDecr:
      return evalPostDecr(*a);
    case Operator::Sub:
      return evalNegative(*a);
    default:
      throw interpreter_error("An unary operator is expected", a->location);
    }
  }
  throw interpreter_error("Cannot recognize the expression type",
                          expr.location);
}

RuntimeValue Interpreter::evalFunctionCall(const FunctionCall &expr) {
  auto previous = environment;
  bool previousinside = insidefunction;
  try {
    auto func = validCheck(environment->getPointer(expr.name), expr.location,
                           expr.name);
    if (func->type != Datatype::Function)
      throw interpreter_error(expr.name + " is not a function to call",
                              expr.location);
    auto realfunc = std::get<std::shared_ptr<Function>>(func->data);
    if (realfunc->declaration->parameters.size() != expr.parameters.size())
      throw interpreter_error(
          "Expected paramters: " +
              std::to_string(realfunc->declaration->parameters.size()) +
              " and given parameters are " +
              std::to_string(expr.parameters.size()),
          expr.location);
    environment = std::make_shared<Environment>(Environment{{}, nullptr});
    insidefunction = true;
    for (size_t i = 0; i < expr.parameters.size(); i++) {
      environment->set(realfunc->declaration->parameters[i],
                       eval(*expr.parameters[i]));
    }
    for (size_t i = 0;
         i < realfunc->declaration->Instructions->statements.size(); i++) {
      matchStatement(*realfunc->declaration->Instructions->statements[i]);
    }
    environment = previous;
    insidefunction = previousinside;
    return RuntimeValue();
  } catch (const ReturnException &exc) {
    environment = previous;
    insidefunction = previousinside;
    return exc.value;
  }
}

bool Interpreter::isNumeric(const RuntimeValue &value) {
  if (value.type == Datatype::Int || value.type == Datatype::Char ||
      value.type == Datatype::Double || value.type == Datatype::Bool)
    return true;
  return false;
}

RuntimeValue Interpreter::convertString(const Cast &expr) {
  try {
    auto b = eval(*expr.expr);
    switch (expr.castTo) {
    case Datatype::Int:
      return {Datatype::Int, std::stoll(std::get<std::string>(b.data))};
    case Datatype::Double:
      return {Datatype::Double, std::stod(std::get<std::string>(b.data))};
    case Datatype::Char:
      if (auto a = std::get<std::string>(b.data); a.size() == 1)
        return {Datatype::Char, static_cast<unsigned char>(a[0])};
      else
        throw std::runtime_error("err");
    case Datatype::Bool:
      if (auto a = std::get<std::string>(b.data); a == "true" || a == "false")
        return {Datatype::Bool, a == "true" ? true : false};
      throw std::runtime_error("err");
    default:
      throw std::runtime_error("err");
    }
  } catch (const std::exception &) {
    throw interpreter_error("The string cannot be casted to another data type",
                            expr.location);
  }
}

double Interpreter::toDouble(const RuntimeValue &value, const Location &loc) {
  switch (value.type) {
  case Datatype::Int:
    return std::get<int64_t>(value.data);
  case Datatype::Double:
    return std::get<double>(value.data);
  case Datatype::Char:
    return std::get<unsigned char>(value.data);
  case Datatype::Bool:
    return std::get<bool>(value.data) ? 1.0 : 0.0;
  default:
    throw interpreter_error("Such data type cannot be casted to double", loc);
  }
}

int64_t Interpreter::toInt(const RuntimeValue &value, const Location &loc) {
  switch (value.type) {
  case Datatype::Int:
    return std::get<int64_t>(value.data);
  case Datatype::Double:
    return static_cast<int64_t>(std::round(std::get<double>(value.data)));
  case Datatype::Char:
    return std::get<unsigned char>(value.data);
  case Datatype::Bool:
    return std::get<bool>(value.data);
  default:
    throw interpreter_error("Such data type cannot be casted to int", loc);
  }
}

std::string Interpreter::toString(const RuntimeValue &value,
                                  const Location &loc) {
  switch (value.type) {
  case Datatype::Int:
    return std::to_string(std::get<int64_t>(value.data));
  case Datatype::Double:
    return std::to_string(std::get<double>(value.data));
  case Datatype::Char:
    return std::string(1, std::get<unsigned char>(value.data));
  case Datatype::Bool:
    return std::get<bool>(value.data) ? "true" : "false";
  default:
    throw interpreter_error("Such data type cannot be casted to string", loc);
  }
}

unsigned char Interpreter::toChar(const RuntimeValue &value,
                                  const Location &loc) {
  int64_t var = toInt(value, loc);
  if (var < 0 || var > 255)
    throw interpreter_error("The value is too big to be casted to char", loc);
  return static_cast<unsigned char>(var);
}

RuntimeValue Interpreter::evalNegative(const Unary &expr) {
  auto value = eval(*expr.expr);
  if (isNumeric(value)) {
    std::visit(
        [](auto &c) {
          using T = std::decay_t<decltype(c)>;
          if constexpr (std::is_arithmetic_v<T>) {
            c = c * (-1);
          }
        },
        value.data);
    return value;
  } else
    throw interpreter_error(
        "A non-numeric data type cannot be used with negative operator",
        expr.location);
}

RuntimeValue Interpreter::evalPreIncr(const Unary &expr) {
  if (auto a = dynamic_cast<const Variable *>(expr.expr.get())) {
    auto b = validCheck(environment->getPointer(a->name), a->location, a->name);
    if (isNumeric(*b)) {
      std::visit(
          [](auto &c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_arithmetic_v<T>) {
              c = c + 1;
            }
          },
          b->data);
      return *b;
    } else
      throw interpreter_error(
          "The increment operator cannot be used to such value type",
          expr.location);
  } else
    throw interpreter_error(
        "The increment operator cannot only be used with variables",
        expr.location);
}

RuntimeValue Interpreter::evalPreDecr(const Unary &expr) {
  if (auto a = dynamic_cast<const Variable *>(expr.expr.get())) {
    auto b = validCheck(environment->getPointer(a->name), a->location, a->name);
    if (isNumeric(*b)) {
      std::visit(
          [](auto &c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_arithmetic_v<T>) {
              c = c - 1;
            }
          },
          b->data);
      return *b;
    } else
      throw interpreter_error(
          "The decrement operator cannot be used to such value type",
          expr.location);
  } else
    throw interpreter_error(
        "The decrement operator cannot only be used with variables",
        expr.location);
}

RuntimeValue Interpreter::evalPostIncr(const Unary &expr) {
  if (auto a = dynamic_cast<const Variable *>(expr.expr.get())) {
    auto b = validCheck(environment->getPointer(a->name), a->location, a->name);
    if (isNumeric(*b)) {
      auto c = *b; // previous value
      std::visit(
          [](auto &c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_arithmetic_v<T>) {
              c = c + 1;
            }
          },
          b->data);
      return c;
    } else
      throw interpreter_error(
          "The increment operator cannot be used to such value type",
          expr.location);
  } else
    throw interpreter_error(
        "The increment operator cannot only be used with variables",
        expr.location);
}

RuntimeValue Interpreter::evalPostDecr(const Unary &expr) {
  if (auto a = dynamic_cast<const Variable *>(expr.expr.get())) {
    auto b = validCheck(environment->getPointer(a->name), a->location, a->name);
    if (isNumeric(*b)) {
      auto c = *b;
      std::visit(
          [](auto &c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_arithmetic_v<T>) {
              c = c - 1;
            }
          },
          b->data);
      return c;
    } else
      throw interpreter_error(
          "The decrement operator cannot be used to such value type",
          expr.location);
  } else
    throw interpreter_error(
        "The decrement operator cannot only be used with variables",
        expr.location);
}

RuntimeValue Interpreter::evalDef(const Binary &expr) {
  if (auto a = dynamic_cast<const Variable *>(expr.left.get())) {
    auto right = eval(*expr.right);
    environment->set(a->name, right);
    return right;
  } else
    throw interpreter_error(
        "The definition operator can only be used to variables", expr.location);
}

RuntimeValue Interpreter::evalAdd(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"+\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) + toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) + toInt(right, loc)};
}

RuntimeValue Interpreter::evalSub(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"-\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) - toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) - toInt(right, loc)};
}

RuntimeValue Interpreter::evalMul(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"*\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) * toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) * toInt(right, loc)};
}

RuntimeValue Interpreter::evalDiv(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"/\" cannot be used to such value type",
                            loc);
  auto DBLright = toDouble(right, loc);
  if (DBLright == 0.0)
    throw interpreter_error("Division by zero is not permitted", loc);
  return {Datatype::Double, toDouble(left, loc) / DBLright};
}

RuntimeValue Interpreter::evalMod(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (left.type != Datatype::Int || right.type != Datatype::Int)
    throw interpreter_error("Operator \"%\" cannot be used to such value type",
                            loc);
  auto INTright = toInt(right, loc);
  if (INTright == 0)
    throw interpreter_error("Division by zero is not permitted", loc);
  return {Datatype::Int, toInt(left, loc) % toInt(right, loc)};
}

RuntimeValue Interpreter::evalGr(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \">\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) > toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) > toInt(right, loc)};
}

RuntimeValue Interpreter::evalLs(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"<\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) < toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) < toInt(right, loc)};
}

RuntimeValue Interpreter::evalGe(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \">=\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) >= toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) >= toInt(right, loc)};
}

RuntimeValue Interpreter::evalLe(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"<=\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) <= toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) <= toInt(right, loc)};
}

RuntimeValue Interpreter::evalEq(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"==\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) == toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) == toInt(right, loc)};
}

RuntimeValue Interpreter::evalNq(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!isNumeric(left) || !isNumeric(right))
    throw interpreter_error("Operator \"!=\" cannot be used to such value type",
                            loc);
  if (left.type == Datatype::Double || right.type == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) != toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) != toInt(right, loc)};
}

bool Interpreter::isTrue(const RuntimeValue &value, const Location &loc) {
  switch (value.type) {
  case Datatype::Int:
    return std::get<int64_t>(value.data) != 0;
  case Datatype::Char:
    return std::get<unsigned char>(value.data) != '\0';
  case Datatype::String:
    return !std::get<std::string>(value.data).empty();
  case Datatype::Double:
    return std::get<double>(value.data) != 0;
  case Datatype::Bool:
    return std::get<bool>(value.data);
  case Datatype::Array:
    return !std::get<std::vector<Value>>(value.data).empty();
  default:
    throw interpreter_error("Such data type cannot be converted to boolean",
                            loc);
  }
}

void Interpreter::expression(const ExpressionStmt &stmt) { eval(*stmt.expr); }

void Interpreter::input(const Input &stmt) {
  if (auto a = dynamic_cast<const Variable *>(stmt.input.get())) {
    std::string str;
    std::cin >> str;
    environment->set(a->name, {Datatype::String, str});
    return;
  } else if (auto a = dynamic_cast<const Cast *>(stmt.input.get())) {
    if (auto b = dynamic_cast<const Variable *>(a->expr.get())) {
      std::string str;
      std::cin >> str;
      environment->set(b->name, {Datatype::String, str});
      environment->set(b->name, convertString(*a));
      return;
    }
  }
  throw interpreter_error(
      "The expressions cannot be used in the input function", stmt.location);
}

void Interpreter::output(const Output &stmt) {
  RuntimeValue value = eval(*stmt.output);
  switch (value.type) {
  case Datatype::Int:
    std::cout << std::get<int64_t>(value.data);
    break;
  case Datatype::Double:
    std::cout << std::get<double>(value.data);
    break;
  case Datatype::Char:
    std::cout << std::get<unsigned char>(value.data);
    break;
  case Datatype::Bool:
    std::cout << std::get<bool>(value.data);
    break;
  case Datatype::String:
    std::cout << std::get<std::string>(value.data);
    break;
  default:
    throw interpreter_error("Such data type cannot be printed", stmt.location);
  }
}

void Interpreter::ifStatement(const IfStatement &stmt) {
  if (isTrue(eval(*stmt.expr), stmt.location)) {
    addScope();
    for (size_t i = 0; i < stmt.Instructions->statements.size(); i++) {
      matchStatement(*stmt.Instructions->statements[i]);
    }
    popScope();
  } else if (stmt.elseStatement) {
    if (stmt.elseStatement->expr)
      ifStatement(*stmt.elseStatement);
    else {
      addScope();
      for (size_t i = 0;
           i < stmt.elseStatement->Instructions->statements.size(); i++) {
        matchStatement(*stmt.elseStatement->Instructions->statements[i]);
      }
      popScope();
    }
  }
}

void Interpreter::whileloop(const While &stmt) {
  while (isTrue(eval(*stmt.expr), stmt.location)) {
    addScope();
    for (size_t i = 0; i < stmt.Instructions->statements.size(); i++) {
      matchStatement(*stmt.Instructions->statements[i]);
    }
    popScope();
  }
}

void Interpreter::forbody(RuntimeValue *&Initial, const short &direction,
                          const For &stmt) {
  addScope();
  for (size_t i = 0; i < stmt.Instructions->statements.size(); i++) {
    matchStatement(*stmt.Instructions->statements[i]);
  }
  popScope();
  Initial = environment->getPointer(stmt.iterator);
  if (stmt.step == nullptr) {
    std::visit(
        [direction](auto &a) {
          using T = std::decay_t<decltype(a)>;
          if constexpr (std::is_arithmetic_v<T>) {
            a += direction;
          }
        },
        Initial->data);
  } else {
    eval(*stmt.step);
  }
}

void Interpreter::forloop(const For &stmt) {
  addScope();
  auto op = stmt.op;
  if (stmt.Initialvalue == nullptr) {
    if (!environment->getPointer(stmt.iterator)) {
      environment->set(stmt.iterator, {Datatype::Int, 0});
    }
  } else {
    if (auto a = environment->getPointer(stmt.iterator))
      *a = eval(*stmt.Initialvalue);
    else
      environment->set(stmt.iterator, eval(*stmt.Initialvalue));
  }
  short direction = -1;
  auto Initial = environment->getPointer(stmt.iterator);
  int64_t Final;
  if (auto a = eval(*stmt.Finalvalue); isNumeric(a) && isNumeric(*Initial)) {
    Final = toInt(a, stmt.location);
  } else
    throw interpreter_error(
        "The initial and the final value both have to be numerical",
        stmt.location);
  if (stmt.op == Operator::Arrow) {
    if (toInt(*Initial, stmt.location) < Final)
      direction = 1;
  } else if (stmt.op == Operator::ArrowEq) {
    if (toInt(*Initial, stmt.location) <= Final)
      direction = 1;
  } else if (stmt.op == Operator::Greater || stmt.op == Operator::Less ||
             stmt.op == Operator::GreaterEq || stmt.op == Operator::LessEq ||
             stmt.op == Operator::NotEqual) {
    direction = 1;
  } else
    throw interpreter_error("Invalid operator", stmt.location);
  if (Initial->type == Datatype::Bool)
    *Initial = {Datatype::Int, toInt(*Initial, stmt.location)};
  switch (op) {
  case Operator::Arrow:
    while ((Final - toInt(*Initial, stmt.location)) * direction > 0) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::ArrowEq:
    while ((Final - toInt(*Initial, stmt.location)) * direction >= 0) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::NotEqual:
    while (toInt(*Initial, stmt.location) != Final) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::Greater:
    while (toInt(*Initial, stmt.location) > Final) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::Less:
    while (toInt(*Initial, stmt.location) < Final) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::GreaterEq:
    while (toInt(*Initial, stmt.location) >= Final) {
      forbody(Initial, direction, stmt);
    }
    break;
  case Operator::LessEq:
    while (toInt(*Initial, stmt.location) <= Final) {
      forbody(Initial, direction, stmt);
    }
    break;
  }
  popScope();
}

void Interpreter::function(const FunctionStatement &stmt) {
  RuntimeValue Func(Datatype::Function,
                    std::make_shared<Function>(Function{&stmt, environment}));
  environment->set(stmt.name, Func);
}

void Interpreter::returnStatement(const ReturnStatement &stmt) {
  if (insidefunction)
    throw ReturnException(stmt.expr ? eval(*stmt.expr) : RuntimeValue());
  else
    throw interpreter_error("Return Statement must be used inside the function",
                            stmt.location);
}

void Interpreter::matchStatement(const Statement &stmt) {
  if (auto a = dynamic_cast<const Output *>(&stmt))
    output(*a);
  else if (auto a = dynamic_cast<const Input *>(&stmt))
    input(*a);
  else if (auto a = dynamic_cast<const ExpressionStmt *>(&stmt))
    expression(*a);
  else if (auto a = dynamic_cast<const IfStatement *>(&stmt))
    ifStatement(*a);
  else if (auto a = dynamic_cast<const While *>(&stmt))
    whileloop(*a);
  else if (auto a = dynamic_cast<const For *>(&stmt))
    forloop(*a);
  else if (auto a = dynamic_cast<const FunctionStatement *>(&stmt))
    function(*a);
  else if (auto a = dynamic_cast<const ReturnStatement *>(&stmt))
    returnStatement(*a);
}

void Interpreter::execute(const Program &program) {
  for (size_t i = 0; i < program.statements.size(); i++) {
    matchStatement(*program.statements[i]);
  }
}
