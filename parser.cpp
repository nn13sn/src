#include "parser.h"
const Token& Parser::peek() const {
    return tokens[line][pos];
}

Token& Parser::advance(){
    if(isEnd()) throw std::invalid_argument("Error: Unexepected Ending.");
    return tokens[line][pos++];
}


Parser::Parser(std::vector <std::vector <Token>>& T) : tokens(T) {}

bool Parser::isEnd(){
    return peek().type == TokenType::End;
}

void Parser::SyntaxErr(){
    throw std::invalid_argument("Error: Invalid Syntax at line: " + std::to_string(peek().lineID) + "; column: " + std::to_string(peek().columnID));
}

bool Parser::Check(TokenType type){
    return type == peek().type;
}

bool Parser::Check(std::string lexeme){
    return lexeme == peek().lexeme;
}

bool Parser::Check(Keyword keyword){
    return keyword == peek().keyword;
}

std::unique_ptr <Expression> Parser::SingleParse(){
    if(Check(TokenType::Number)){
        auto expr = std::make_unique <Number> ();
        expr->number = std::stoi(peek().lexeme);
        advance();
        return expr;
    }
    else if(Check(TokenType::Identifier)){
        auto expr = std::make_unique <Variable> ();
        expr->name = peek().lexeme;
        advance();
        return expr;
    }
    SyntaxErr();
    return nullptr;
}

std::unique_ptr <Expression> Parser::ParseTerm(){
    auto expr = SingleParse();

    while(Check("*") || Check("/")){
        char sign = peek().lexeme[0];
        advance();
        auto right = SingleParse();
        auto left = std::move(expr);
        auto bin = std::make_unique <Binary> ();
        bin->op = sign;
        bin->right = std::move(right);
        bin->left = std::move(left);
        expr = std::move(bin);
    }
    return expr;
}

std::unique_ptr <Expression> Parser::MakeExpression(){
    auto expr = ParseTerm();

    while(Check("+") || Check("-")){
        char sign = peek().lexeme[0];
        advance();
        auto right = ParseTerm();
        auto left = std::move(expr);
        auto bin = std::make_unique <Binary> ();
        bin->op = sign;
        bin->right = std::move(right);
        bin->left = std::move(left);
        expr = std::move(bin);
    }
    return expr;
}

std::unique_ptr <Statement> Parser::MakeStatement(){
    if (Check(Keyword::Int)){
        auto stmt = std::make_unique<Declaration> ();
        if(advance().keyword == Keyword::Int) stmt->type = Datatype::Int;
            if (Check(TokenType::Identifier)) stmt->name = advance().lexeme;
            else SyntaxErr();
            if (Check(";")){
                    advance();
                    return stmt;
            }
            else SyntaxErr();
    }
    else if(Check(Keyword::Out)){
        auto stmt = std::make_unique<Output> ();
        advance();
        if (Check("(")) advance();
        else SyntaxErr();
        if (Check(TokenType::Number) || Check(TokenType::Identifier)) stmt->output = MakeExpression();
        else SyntaxErr();
        if (Check(")")) advance();
        else SyntaxErr();
        if (Check(";")) {
            advance();
            return stmt;
        }
        else SyntaxErr();
    }
    else if (Check(TokenType::Identifier)){
        auto stmt = std::make_unique<Definition> ();
        stmt->name = advance().lexeme;
        if (Check("=")) advance();
        else SyntaxErr();
        stmt->value = MakeExpression();
        if(Check(";")) {
            advance();
            return stmt;
        }
        else SyntaxErr();
    }
    return nullptr; //пока что не буду парсить остальное
}

void Parser::Parse(Program& program){
    try{
    while(line<tokens.size()){
         program.statements.push_back(MakeStatement());
        if(peek().type == TokenType::End){
            line++;
            pos = 0; //изменяю pos только в случае новой строки, тк в одной строке может быть неск инструкций
        }
    }
}
    catch (const std::invalid_argument& e){
        std::cerr <<e.what()<<std::endl;
    }
}
