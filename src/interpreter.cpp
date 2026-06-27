#include "interpreter.h"
#include "AST.h"
#include "runtime_variable.h"
#include "utils.h"
#include <memory>

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
  if (value.getType() == Datatype::Invalid) {
    throw interpreter_error("Undefined variable: " + name, loc);
  }
  return value;
}

RuntimeVariable *Interpreter::validCheck(RuntimeVariable *ptr,
                                         const Location &loc,
                                         const std::string &name) {
  if (!ptr) {
    throw interpreter_error("Undefined variable: " + name, loc);
  }
  return ptr;
}

RuntimeValue Interpreter::eval(const Expression &expr) {
  switch (expr.ExpressionType) {
  case ExprType::exprValue:
    return static_cast<const exprValue &>(expr).value;
  case ExprType::Variable: {
    auto var = static_cast<const Variable &>(expr);
    return validCheck(environment->get(var.name).get(), var.location, var.name);
  }
  case ExprType::FunctionCall:
    return evalFunctionCall(static_cast<const FunctionCall &>(expr));
  case ExprType::Binary: {
    const auto &a = static_cast<const Binary &>(expr);
    switch (a.op) {
    case Operator::Add:
      return evalAdd(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Sub:
      return evalSub(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Mul:
      return evalMul(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Div:
      return evalDiv(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Mod:
      return evalMod(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Greater:
      return evalGr(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Less:
      return evalLs(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Equal:
      return evalEq(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::NotEqual:
      return evalNq(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::GreaterEq:
      return evalGe(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::LessEq:
      return evalLe(eval(*(a.left)), eval(*(a.right)), a.location);
    case Operator::Def:
      return evalDef(a);
    case Operator::AND:
      if (!isTrue(eval(*(a.left)), a.location))
        return {Datatype::Bool, false};
      if (!isTrue(eval(*(a.right)), a.location))
        return {Datatype::Bool, false};
      return {Datatype::Bool, true};
    case Operator::OR:
      if (isTrue(eval(*(a.left)), a.location))
        return {Datatype::Bool, true};
      if (isTrue(eval(*(a.right)), a.location))
        return {Datatype::Bool, true};
      return {Datatype::Bool, false};
    default:
      throw interpreter_error("A binary operator is expected", a.location);
    }
  }
  case ExprType::Cast: {
    const auto &cast = static_cast<const Cast &>(expr);
    auto b = eval(*cast.expr);
    if (b.getType() == Datatype::String)
      return convertString(cast);
    switch (cast.castTo) {
    case Datatype::Int:
      return {Datatype::Int, toInt(b, cast.location)};
    case Datatype::Double:
      return {Datatype::Double, toDouble(b, cast.location)};
    case Datatype::Char:
      return {Datatype::Char, toChar(b, cast.location)};
    case Datatype::Bool:
      return {Datatype::Bool, isTrue(b, cast.location)};
    case Datatype::String:
      return {Datatype::String, toString(b, cast.location)};
    default:
      throw interpreter_error("Invalid data type to be casted to",
                              cast.location);
    }
  }
  case ExprType::Unary: {
    const auto &unary = static_cast<const Unary &>(expr);
    switch (unary.op) {
    case Operator::Not:
      return {Datatype::Bool, !isTrue(eval(*unary.expr), unary.location)};
    case Operator::PreIncr:
      return evalPreIncr(unary);
    case Operator::PreDecr:
      return evalPreDecr(unary);
    case Operator::PostIncr:
      return evalPostIncr(unary);
    case Operator::PostDecr:
      return evalPostDecr(unary);
    case Operator::Sub:
      return evalNegative(unary);
    default:
      throw interpreter_error("An unary operator is expected", unary.location);
    }
  }
  default:
    throw interpreter_error("Cannot recognize the expression type",
                            expr.location);
  }
}

RuntimeValue Interpreter::evalFunctionCall(const FunctionCall &expr) {
  auto previous = environment;
  bool previousinside = insidefunction;
  try {
    auto func = validCheck(environment->getPointer(expr.name), expr.location,
                           expr.name);
    if (func->get().getType() != Datatype::Function)
      throw interpreter_error(expr.name + " is not a function to call",
                              expr.location);
    auto realfunc = std::get<std::shared_ptr<Function>>(func->get().getData());
    if (realfunc->declaration->params.size() != expr.parameters.size())
      throw interpreter_error(
          "Expected number of parameters is: " +
              std::to_string(realfunc->declaration->params.size()) +
              ", and the given number of parameters is: " +
              std::to_string(expr.parameters.size()),
          expr.location);
    if (utils::isDynamic(realfunc->declaration->mods))
      addScope();
    else
      environment = std::make_shared<Environment>(Environment{{}, nullptr});
    insidefunction = true;
    for (size_t i = 0; i < expr.parameters.size(); i++) {
      environment->set(realfunc->declaration->params[i].name,
                       eval(*expr.parameters[i]), expr.location,
                       realfunc->declaration->params[i].mods);
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

RuntimeValue Interpreter::convertString(const Cast &expr) {
  try {
    auto b = eval(*expr.expr);
    switch (expr.castTo) {
    case Datatype::Int:
      return {Datatype::Int, std::stoll(std::get<std::string>(b.getData()))};
    case Datatype::Double:
      return {Datatype::Double, std::stod(std::get<std::string>(b.getData()))};
    case Datatype::Char:
      if (auto a = std::get<std::string>(b.getData()); a.size() == 1)
        return {Datatype::Char, static_cast<unsigned char>(a[0])};
      else
        throw std::runtime_error("err");
    case Datatype::Bool:
      if (auto a = std::get<std::string>(b.getData());
          a == "true" || a == "false")
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
  switch (value.getType()) {
  case Datatype::Int:
    return std::get<int64_t>(value.getData());
  case Datatype::Double:
    return std::get<double>(value.getData());
  case Datatype::Char:
    return std::get<unsigned char>(value.getData());
  case Datatype::Bool:
    return std::get<bool>(value.getData()) ? 1.0 : 0.0;
  default:
    throw interpreter_error("Such data type cannot be casted to double", loc);
  }
}

int64_t Interpreter::toInt(const RuntimeValue &value, const Location &loc) {
  switch (value.getType()) {
  case Datatype::Int:
    return std::get<int64_t>(value.getData());
  case Datatype::Double:
    return static_cast<int64_t>(std::round(std::get<double>(value.getData())));
  case Datatype::Char:
    return std::get<unsigned char>(value.getData());
  case Datatype::Bool:
    return std::get<bool>(value.getData());
  default:
    throw interpreter_error("Such data type cannot be casted to int", loc);
  }
}

std::string Interpreter::toString(const RuntimeValue &value,
                                  const Location &loc) {
  switch (value.getType()) {
  case Datatype::Int:
    return std::to_string(std::get<int64_t>(value.getData()));
  case Datatype::Double:
    return std::to_string(std::get<double>(value.getData()));
  case Datatype::Char:
    return std::string(1, std::get<unsigned char>(value.getData()));
  case Datatype::Bool:
    return std::get<bool>(value.getData()) ? "true" : "false";
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
  if (utils::isNumerical(value)) {
    std::visit(
        [&](const auto &c) {
          using T = std::decay_t<decltype(c)>;
          if constexpr (std::is_arithmetic_v<T>) {
            value.setData(c * (-1), expr.location);
          }
        },
        value.getData());
    return value;
  } else
    throw interpreter_error(
        "A non-numeric data type cannot be used with negative operator",
        expr.location);
}

RuntimeValue Interpreter::evalPreIncr(const Unary &expr) {
  const auto &a = static_cast<const Variable &>(*expr.expr);
  auto b = validCheck(environment->getPointer(a.name), a.location, a.name);
  b->increment(expr.location);
  return b->get();
}

RuntimeValue Interpreter::evalPreDecr(const Unary &expr) {
  const auto &a = static_cast<const Variable &>(*expr.expr);
  auto b = validCheck(environment->getPointer(a.name), a.location, a.name);
  b->decrement(expr.location);
  return b->get();
}

RuntimeValue Interpreter::evalPostIncr(const Unary &expr) {
  const auto &a = static_cast<const Variable &>(*expr.expr);
  auto b = validCheck(environment->getPointer(a.name), a.location, a.name);
  auto c = *b;
  b->increment(expr.location);
  return c.get();
}

RuntimeValue Interpreter::evalPostDecr(const Unary &expr) {
  const auto &a = static_cast<const Variable &>(*expr.expr);
  auto b = validCheck(environment->getPointer(a.name), a.location, a.name);
  auto c = *b;
  b->decrement(expr.location);
  return c.get();
}

RuntimeValue Interpreter::evalDef(const Binary &expr) {
  const auto &a = static_cast<const Variable &>(*expr.left);
  auto right = eval(*expr.right);
  environment->set(a.name, right, expr.location, currentmods);
  return right;
}

RuntimeValue Interpreter::evalAdd(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"+\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) + toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) + toInt(right, loc)};
}

RuntimeValue Interpreter::evalSub(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"-\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) - toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) - toInt(right, loc)};
}

RuntimeValue Interpreter::evalMul(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"*\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Double, toDouble(left, loc) * toDouble(right, loc)};
  return {Datatype::Int, toInt(left, loc) * toInt(right, loc)};
}

RuntimeValue Interpreter::evalDiv(const RuntimeValue &left,
                                  const RuntimeValue &right,
                                  const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
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
  if (left.getType() != Datatype::Int || right.getType() != Datatype::Int)
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
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \">\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) > toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) > toInt(right, loc)};
}

RuntimeValue Interpreter::evalLs(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"<\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) < toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) < toInt(right, loc)};
}

RuntimeValue Interpreter::evalGe(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \">=\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) >= toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) >= toInt(right, loc)};
}

RuntimeValue Interpreter::evalLe(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"<=\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) <= toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) <= toInt(right, loc)};
}

RuntimeValue Interpreter::evalEq(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"==\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) == toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) == toInt(right, loc)};
}

RuntimeValue Interpreter::evalNq(const RuntimeValue &left,
                                 const RuntimeValue &right,
                                 const Location &loc) {
  if (!utils::isNumerical(left) || !utils::isNumerical(right))
    throw interpreter_error("Operator \"!=\" cannot be used to such value type",
                            loc);
  if (left.getType() == Datatype::Double || right.getType() == Datatype::Double)
    return {Datatype::Bool, toDouble(left, loc) != toDouble(right, loc)};
  return {Datatype::Bool, toInt(left, loc) != toInt(right, loc)};
}

bool Interpreter::isTrue(const RuntimeValue &value, const Location &loc) {
  switch (value.getType()) {
  case Datatype::Int:
    return std::get<int64_t>(value.getData()) != 0;
  case Datatype::Char:
    return std::get<unsigned char>(value.getData()) != '\0';
  case Datatype::String:
    return !std::get<std::string>(value.getData()).empty();
  case Datatype::Double:
    return std::get<double>(value.getData()) != 0;
  case Datatype::Bool:
    return std::get<bool>(value.getData());
  case Datatype::Array:
    return !std::get<std::vector<Literal>>(value.getData()).empty();
  default:
    throw interpreter_error("Such data type cannot be converted to boolean",
                            loc);
  }
}

void Interpreter::expression(const ExpressionStmt &stmt) { eval(*stmt.expr); }

void Interpreter::input(const Input &stmt) {
  if (stmt.input->ExpressionType == ExprType::Variable) {
    const auto &a = static_cast<const Variable &>(*stmt.input);
    std::string str;
    std::cin >> str;
    environment->set(a.name, {Datatype::String, str}, a.location, currentmods);
    return;
  } else if (stmt.input->ExpressionType == ExprType::Cast) {
    const auto &a = static_cast<const Cast &>(*stmt.input);
    if (a.expr->ExpressionType == ExprType::Variable) {
      const auto &b = static_cast<const Variable &>(*a.expr);
      std::string str;
      std::cin >> str;
      environment->set(b.name, {Datatype::String, str}, b.location,
                       currentmods);
      environment->set(b.name, convertString(a), b.location, currentmods);
      return;
    }
  }
}

void Interpreter::output(const Output &stmt) {
  RuntimeValue value = eval(*stmt.output);
  switch (value.getType()) {
  case Datatype::Int:
    std::cout << std::get<int64_t>(value.getData());
    break;
  case Datatype::Double:
    std::cout << std::get<double>(value.getData());
    break;
  case Datatype::Char:
    std::cout << std::get<unsigned char>(value.getData());
    break;
  case Datatype::Bool:
    std::cout << std::get<bool>(value.getData());
    break;
  case Datatype::String:
    std::cout << std::get<std::string>(value.getData());
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

void Interpreter::block(const BlockStatement &stmt) {
  addScope();
  for (const auto &instr : stmt.instructions->statements) {
    matchStatement(*instr);
  }
  popScope();
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

bool Interpreter::getCond(const int64_t &Initial, const int64_t Final,
                          const signed char &direction, const Operator &op) {
  switch (op) {
  case Operator::Arrow:
    return (Final - Initial) * direction > 0;
  case Operator::ArrowEq:
    return (Final - Initial) * direction >= 0;
  case Operator::NotEqual:
    return Initial != Final;
  case Operator::Greater:
    return Initial > Final;
  case Operator::Less:
    return Initial < Final;
  case Operator::GreaterEq:
    return Initial >= Final;
  case Operator::LessEq:
    return Initial <= Final;
  default:
    return false;
  }
}

void Interpreter::forloop(const For &stmt) {
  addScope();
  auto op = stmt.op;
  int64_t Final;
  int64_t step = -1;

  if (stmt.Initialvalue == nullptr) {
    if (!environment->getPointer(stmt.iterator)) {
      environment->set(stmt.iterator, {Datatype::Int, 0}, stmt.location,
                       stmt.mods);
    }
  } else {
    if (auto a = environment->getPointer(stmt.iterator))
      a->set(eval(*stmt.Initialvalue), stmt.location);
    else
      environment->set(stmt.iterator, eval(*stmt.Initialvalue),
                       stmt.Initialvalue->location, stmt.mods);
  }

  signed char direction = -1;
  auto Initial = environment->getPointer(stmt.iterator);
  if (auto a = eval(*stmt.Finalvalue);
      a.getType() == Datatype::Int &&
      Initial->get().getType() == Datatype::Int) {
    Final = std::get<int64_t>(a.getData());
  } else
    throw interpreter_error("The for loop requires integer bounds",
                            stmt.location);
  auto localIterator = std::get<int64_t>(Initial->get().getData());

  if (stmt.op == Operator::Arrow) {
    if (localIterator < Final)
      direction = step = 1;
  } else if (stmt.op == Operator::ArrowEq) {
    if (localIterator <= Final)
      direction = step = 1;
  } else {
    direction = step = 1;
  }
  if (stmt.step && !utils::isDynamic(stmt.mods)) {
    auto a = eval(*stmt.step);
    if (a.getType() == Datatype::Int)
      step = std::get<int64_t>(a.getData());
    else
      throw interpreter_error("The for loop requires integer step",
                              stmt.location);
  }

  if (utils::isDynamic(stmt.mods)) {
    while (getCond(std::get<int64_t>(Initial->get().getData()), Final,
                   direction, op)) {
      addScope();
      execute(*stmt.Instructions);
      popScope();
      if (Initial->get().getType() != Datatype::Int)
        // in case if the iterator was changed to another data type inside the
        // loop
        throw interpreter_error("The iterator must stay integer",
                                stmt.location);
      if (stmt.step) {
        eval(*stmt.step);
        if (Initial->get().getType() != Datatype::Int)
          throw interpreter_error("The iterator must stay integer",
                                  stmt.location);
        // in case if the step can change the data type of an iterator
      } else {
        Initial->setData(std::get<int64_t>(Initial->get().getData()) + step,
                         stmt.location);
      }
      if (auto a = eval(*stmt.Finalvalue); a.getType() == Datatype::Int)
        Final = std::get<int64_t>(a.getData());
      else
        throw interpreter_error("The boundaries has to stay integer",
                                stmt.Finalvalue->location);
    }
  } else {
    Initial->lock();
    while (getCond(localIterator, Final, direction, op)) {
      addScope();
      execute(*stmt.Instructions);
      popScope();
      localIterator += step;
      Initial->get().setData(localIterator, stmt.location);
    }
    Initial->unlock();
  }
  popScope();
}

void Interpreter::function(const FunctionStatement &stmt) {
  RuntimeValue Func(Datatype::Function,
                    std::make_shared<Function>(Function{&stmt}));
  if (!environment->parent && !environment->getPointer(stmt.name))
    currentmods |= MOD_GLOBAL;
  environment->set(stmt.name, Func, stmt.location, currentmods);
}

void Interpreter::returnStatement(const ReturnStatement &stmt) {
  if (insidefunction)
    throw ReturnException(stmt.expr ? eval(*stmt.expr) : RuntimeValue());
  else
    throw interpreter_error(
        "The return statement must be used inside the function", stmt.location);
}

void Interpreter::matchStatement(const Statement &stmt) {
  currentmods = stmt.mods;
  switch (stmt.StatementType) {
  case StmtType::Output:
    return output(static_cast<const Output &>(stmt));
  case StmtType::Input:
    return input(static_cast<const Input &>(stmt));
  case StmtType::ExpressionStmt:
    return expression(static_cast<const ExpressionStmt &>(stmt));
  case StmtType::IfStatement:
    return ifStatement(static_cast<const IfStatement &>(stmt));
  case StmtType::While:
    return whileloop(static_cast<const While &>(stmt));
  case StmtType::For:
    return forloop(static_cast<const For &>(stmt));
  case StmtType::FunctionStatement:
    return function(static_cast<const FunctionStatement &>(stmt));
  case StmtType::ReturnStatement:
    return returnStatement(static_cast<const ReturnStatement &>(stmt));
  case StmtType::BlockStatement:
    return block(static_cast<const BlockStatement &>(stmt));
  default:
    throw interpreter_error("Unknown Statement type", stmt.location);
  }
}

void Interpreter::execute(const Program &program) {
  for (size_t i = 0; i < program.statements.size(); i++) {
    matchStatement(*program.statements[i]);
  }
}
