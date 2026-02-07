#include "interpreter.h"

int Interpreter::eval(const Expression& expr){
  if(auto a = dynamic_cast<const Number*> (&expr)) return a->number;
  else if (auto a = dynamic_cast<const Variable*> (&expr)) {
    if(auto b = findVar(a->name)) return *b;
    else throw std::invalid_argument("Error: No such variable.");
  }
  else if (auto a = dynamic_cast<const Logical*> (&expr)){
    if(a->op == ">") return eval(*(a->left)) > eval(*(a -> right)); 
    else if(a->op == "<") return eval(*(a->left)) < eval(*(a -> right));
    else if(a->op == "==") return eval(*(a->left)) == eval(*(a -> right));
    else if(a->op == ">=") return eval(*(a->left)) >= eval(*(a -> right));
    else if(a->op == "<=") return eval(*(a->left)) <= eval(*(a -> right));
    else throw std::invalid_argument("Error: Invalid logical operator.");
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

int* Interpreter::findVar(const std::string& name){
  for (auto i = variables.rbegin(); i != variables.rend();i++){
    auto found = i->find(name);
    if(found != i->end()) return &found -> second;
  }
  return nullptr;
}

void Interpreter::definition(const Definition& stmt){
  if(int* a = findVar(stmt.name)) *a= eval(*stmt.value); 
  else throw std::invalid_argument("Error: no such value");
}

void Interpreter::declaration(const Declaration& stmt){
  variables.back()[stmt.name] = 0;
}

void Interpreter::output(const Output& stmt){
  std::cout << eval(*stmt.output);
}

void Interpreter::ifStatement(const IfStatement& stmt){
  if(eval(*stmt.expr)) {
    variables.push_back({});
    for(size_t i = 0; i < stmt.Instructions->statements.size(); i++){
      matchStatement(*stmt.Instructions->statements[i]);
    }
  variables.pop_back();
  }
  else if(stmt.elseStatement){
     if(stmt.elseStatement->expr) ifStatement(*stmt.elseStatement);
     else {
       variables.push_back({});
       for(size_t i = 0; i < stmt.elseStatement->Instructions->statements.size(); i++){
         matchStatement(*stmt.elseStatement->Instructions->statements[i]);
         }
       variables.pop_back();
     }
  } 
}

void Interpreter::matchStatement(const Statement& stmt){
  if(auto a = dynamic_cast<const Declaration*> (&stmt)) declaration(*a);
  else if (auto a = dynamic_cast<const Output*> (&stmt)) output(*a);
  else if (auto a = dynamic_cast<const Definition*> (&stmt)) definition(*a);
  else if (auto a = dynamic_cast<const IfStatement*> (&stmt)) ifStatement(*a);
}

void Interpreter::execute(const Program& program){
  try{
    variables.push_back({});
    for(size_t i = 0; i < program.statements.size(); i++){
      matchStatement(*program.statements[i]);
    }
  }
  catch(const std::invalid_argument& e){
    std::cerr << e.what()<<std::endl;
  }
}
