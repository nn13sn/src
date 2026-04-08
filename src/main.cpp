#include "AST.h"
#include "analyzer.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    if (argc == 1) {
      // std::cout << "The path is expected to be provided\n";
      // return -4;
    }
    std::string path = "ExampleCode.txt";
    Program program;
    Lexer lexer;
    lexer.readFile(path);
    auto tokens = lexer.Tokenize();
    Parser parser(tokens);
    parser.Parse(program);
    Analyzer analyzer;
    if (analyzer.analyze(program) == AnalyzerError) {
      analyzer.printErrors();
      return -5;
    }
    Interpreter interpreter;
    interpreter.execute(program);
    return 0;
  } catch (const std::invalid_argument &err) {
    std::cerr << "Syntax error: " << err.what() << std::endl;
    return -1;
  } catch (const interpreter_error &err) {
    std::cerr << "Runtime error: " << err.what()
              << " at line: " + std::to_string(err.location.line);
    if (err.location.column != 0)
      std::cerr << "; column: " + std::to_string(err.location.column);
    std::cerr << std::endl;
    return -2;
  } catch (const std::runtime_error &err) {
    std::cerr << "Runtime error: " << err.what() << std::endl;
    return -3;
  }
  return 0;
}
