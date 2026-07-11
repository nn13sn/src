#include "AST.h"
#include "VM.h"
#include "analyzer.h"
#include "generator.h"
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
    if (parser.Parse(program) == PARSER_ERROR) {
      parser.printErrors();
      return -3;
    }
    Analyzer analyzer;
    if (analyzer.analyze(program) == AnalyzerError) {
      analyzer.printErrors();
      return -5;
    }
    // Interpreter interpreter;
    // interpreter.execute(program);
    Generator generator;
    auto code = generator.Generate(program);
    VM vm;
    auto result = vm.evaluate(code);
    if (result == VM_ERROR)
      return -2;
    return 0;
  } catch (const std::invalid_argument &err) {
    std::cerr << "Lexer error: " << err.what() << std::endl;
    return -1;
  } catch (const interpreter_error &err) {
    std::cerr << "\nRuntime error: " << err.what()
              << " at line: " + std::to_string(err.location.line);
    std::cerr << "; column: " + std::to_string(err.location.column) << '\n';
    return -2;
  }
  return 0;
}
