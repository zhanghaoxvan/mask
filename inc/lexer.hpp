#pragma once
#include "inc.hpp"

enum class TokenType {
    // 关键字
    KwIf, KwElse, KwWhile, KwFor, KwReturn, KwVar, KwVal, 
    KwTrue, KwFalse, KwConstructor, KwStruct, KwInterface,
    KwImpl, KwFunc, KwModule, KwImport, KwOperator, KwThis,
    KwIRLib, KwPrivate, KwPublic,
    
    // 字面量
    Identifier, Number, Str,
    
    // 运算符
    Plus, Minus, Star, Slash, Assign, Equal, NotEqual, 
    Less, Greater, LessEqual, GreaterEqual, ReturnType,
    And, Or, Not,
    
    // 分隔符
    Comma, Semicolon, LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Colon, Dot, Tilde,
    
    // 特殊
    Eof, Error, Top // Top是整个文件的主节点
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    
    Token(TokenType t, std::string l, int ln);
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> scanTokens();
    
private:
    void initKeywords();
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    void skipWhitespaceAndComments();
    Token scanToken();
    Token stringToken();
    Token identifierToken();
    Token numberToken();
    
private:
    const std::string source;
    size_t position = 0;
    int line = 1;
    std::unordered_map<std::string, TokenType> keywords;
};
