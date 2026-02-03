#include "lexer.h"

Token::Token(TokenType type, const Keyword& keyword, std::string lexeme,size_t lineID, size_t columnID){
    this->type=type;
    this->keyword=keyword;
    this->lexeme=lexeme;
    this->lineID=lineID;
    this->columnID=columnID;
} 

std::string_view readIdentifier(std::string_view InitLine, size_t &pos){
    size_t startpos = pos;
     while(pos<InitLine.size() && (std::isalpha(InitLine[pos]) || InitLine[pos]=='_')) {
        pos++;
    }
    return std::string_view(InitLine.data()+startpos, InitLine.data()+pos);
}

Keyword IsKeyword(const std::string_view lexeme){
    static constexpr std::array <std::string_view, static_cast <size_t> (Keyword::amount)> keywords {
        "if", "else", "int", "in", "out"
    };
    for(size_t i=0; i<keywords.size();i++){
        if (lexeme==keywords[i]) return static_cast<Keyword>(i);
    }
    return Keyword::amount;
}

std::vector <std::vector <Token>> Tokenize(std::vector <std::string>& Initialcode){
   size_t pos = 0;
   Keyword result;
   std::string_view lexeme;
   std::vector <Token> tokens;
   std::vector <std::vector <Token>> alltokens;
   for(size_t i = 0; i < Initialcode.size(); i++){
     std::string InitLine = Initialcode[i];
    while(pos<InitLine.size()){
        if(InitLine[pos]==' ') {
            pos++;
            continue;
        }
        else if(std::isalpha(InitLine[pos]) || InitLine[pos]=='_') {
            lexeme = readIdentifier(InitLine, pos);
            result = IsKeyword(lexeme);
            if(result != Keyword::amount) tokens.emplace_back(TokenType::Keyword, result, std::string(lexeme), i, static_cast<size_t> (pos));
            else tokens.emplace_back(TokenType::Identifier, result, std::string(lexeme), i, static_cast<size_t> (pos));
        }
        else if (std::isdigit(InitLine[pos])) 
        {
            tokens.emplace_back(TokenType::Number, Keyword::amount, std::string(1, InitLine[pos]), i, static_cast<size_t> (pos));
            pos++;
        }
        else 
        {
            switch(InitLine[pos]){
                case '(':
                case ')':
                case ':':
                case ';':
                case '[':
                case ']':
                tokens.emplace_back(TokenType::Separator, Keyword::amount, std::string(1, InitLine[pos]), i, static_cast<size_t> (pos));
                break;
                default:
                tokens.emplace_back(TokenType::Operator, Keyword::amount, std::string(1, InitLine[pos]), i, static_cast<size_t> (pos));
                break;
            }
            pos++;
        }
    }
    tokens.emplace_back(TokenType::End, Keyword::amount, "", i, static_cast<size_t> (pos));
    alltokens.push_back(tokens);
    tokens.clear();
    pos = 0;
   }
    return alltokens;
}


