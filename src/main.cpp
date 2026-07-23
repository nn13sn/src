#define DC_OK 0
#define DC_LEXER_ERROR -1
#define DC_PARSER_ERROR -2
#define DC_ANALYZER_ERROR -3
#define DC_RUNTIME_ERROR -4
#define DC_FILEPATH_ERROR -5

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
      // return DC_FILEPATH_ERROR;
    }
    std::string path = "ExampleCode.txt";
    Program program;
    Lexer lexer;
    lexer.readFile(path);
    auto tokens = lexer.Tokenize();
    Parser parser(tokens);
    if (parser.Parse(program) == PARSER_ERROR) {
      parser.printErrors();
      return DC_PARSER_ERROR;
    }
    tokens.clear();
    std::cout << "Reached Analyzer!\n";
    Analyzer analyzer;
    if (analyzer.analyze(program) == AnalyzerError) {
      analyzer.printErrors();
      return DC_ANALYZER_ERROR;
    }
    // Interpreter interpreter;
    // interpreter.execute(program);
    std::cout << "Reached Generator!\n";
    Generator generator(analyzer.table);
    auto code = generator.Generate(program);
    // program.statements.clear();
    std::cout << "Reached VM!\n";
    VM vm(analyzer.table.slots.size());
    auto result = vm.evaluate(code);
    if (result == VM_ERROR)
      return DC_RUNTIME_ERROR;
    return DC_OK;
  } catch (const std::invalid_argument &err) {
    std::cerr << "Lexer error: " << err.what() << std::endl;
    return DC_LEXER_ERROR;
  }
  return DC_OK;
}
