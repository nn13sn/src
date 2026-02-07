#include <iostream>
#include "lexer.h"
#include "AST.h"
#include "parser.h"
#include "interpreter.h"

int main(){
    Program program;
    std::vector <std::string> code = readFile("code.txt");
    //std::cout<<"hey\n";
    //code.push_back("int value;");
    //code.push_back("value = 1 + 7;");
    //code.push_back("out(value /2+9);");
    std::vector <std::vector <Token>> tokens = Tokenize(code);
    Parser parser(tokens);
    parser.Parse(program);
    Interpreter interpreter;
    interpreter.execute(program);
    return 0;

}
