#include "interpreter.h"

int Interpreter::eval(const Expression& expr){
  if(auto a = dynamic_cast<const Number*> (&expr)) return a->number;
  else if (auto a = dynamic_cast<const Variable*> (&expr)) {
    if(variables.contains(a->name)) return variables[a->name];
    else throw std::invalid_argument("Error: No such variable.");
  }
  else if (auto a = dynamic_cast<const Binary*> (&expr)) {
    switch(a->op){
      case '+':
        return eval(*(a->left)) + eval(*(a->right));
        break;
      case '-':
        return eval(*(a->left)) - eval(*(a->right));
        break;
      case '*':
        return eval(*(a->left)) * eval(*(a->right));
        break;
      case '/':
        return eval(*(a->left)) / eval(*(a->right));
        break;
      case '%':
        return eval(*(a->left)) % eval(*(a->right));
        break;
      default:
        throw std::invalid_argument("Error: No such operator.");
    }
  }
  return 0;
}

void Interpreter::definition(const Definition& stmt){
  if(variables.contains(stmt.name)) {
    variables[stmt.name] = eval(*stmt.value);
  }
  else throw std::invalid_argument("Error: no such value");
}

void Interpreter::declaration(const Declaration& stmt){
  variables[stmt.name] = 0;
}

void Interpreter::output(const Output& stmt){
  std::cout << eval(*stmt.output);
}

void Interpreter::matchStatement(const Statement& stmt){
  if(auto a = dynamic_cast<const Declaration*> (&stmt)) declaration(*a);
  else if (auto a = dynamic_cast<const Output*> (&stmt)) output(*a);
  else if (auto a = dynamic_cast<const Definition*> (&stmt)) definition(*a);
}

void Interpreter::execute(const Program& program){
  try{
    for(size_t i = 0; i < program.statements.size(); i++){
      matchStatement(*program.statements[i]);
    }
  }
  catch(const std::invalid_argument& e){
    std::cerr << e.what()<<std::endl;
  }
}
