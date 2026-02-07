#pragma once
#include <iostream>
#include <stdexcept>
#include <cstddef>
#include "lexer.h"
#include "AST.h"
class Parser {
    private:
    size_t line = 0;
    size_t pos = 0;
    void SyntaxErr();
    const Token& peek() const;
    Token& advance();
    std::vector <std::vector <Token>>& tokens;
    std::unique_ptr <Statement> MakeStatement();
    std::unique_ptr <Statement> ParseDeclaration();
    std::unique_ptr <Statement> ParseOutput();
    std::unique_ptr <Statement> ParseDefinition();
    std::unique_ptr <Statement> ParseIfStatement();
    std::unique_ptr <Expression> ParseMidTerm();
    std::unique_ptr <Expression> MakeExpression();
    std::unique_ptr <Expression> ParseTerm();
    std::unique_ptr <Expression> SingleParse();
    std::unique_ptr <Program> MakeBody();
    bool Check(TokenType type);
    bool Check(std::string lexeme);
    bool Check(Keyword keyword);
    bool isEnd();
    bool eatEnd();
    public:
    Parser(std::vector <std::vector <Token>>& T);
    void Parse(Program& program);
};
