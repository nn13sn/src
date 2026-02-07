#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <string_view>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <fstream>

enum class TokenType{
    Identifier,
    Keyword,
    Number, 
    String,
    Operator,
    Separator,
    End,
    Invalid
};

enum class Keyword : uint8_t {
    If,
    Else, 
    Int, 
    In,
    Out,
    amount
};

struct Token{
    TokenType type;
    Keyword keyword;
    std::string lexeme;
    size_t lineID;
    size_t columnID;
    Token(TokenType type, const Keyword& keyword, std::string lexeme,size_t lineID, size_t columnID);
};

std::string_view readIdentifier(std::string_view InitLine, size_t &pos);

Keyword IsKeyword(const std::string_view lexeme);

std::vector <std::vector <Token>> Tokenize(std::vector <std::string>& InitLine);

std::vector <std::string> readFile(std::string name);
