#include "lexer.hpp"
#include <cctype>

Token::Token(TokenType t, std::string l, int ln)
    : type(t), lexeme(std::move(l)), line(ln) { }

Lexer::Lexer(std::string source) 
    : source(std::move(source)) {
    initKeywords();
}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        tokens.push_back(scanToken());
    }
    tokens.emplace_back(TokenType::Eof, "", line);
    return tokens;
}

void Lexer::initKeywords() {
    keywords = {
        { "if",          TokenType::KwIf },
        { "else",        TokenType::KwElse },
        { "while",       TokenType::KwWhile },
        { "for",         TokenType::KwFor },
        { "return",      TokenType::KwReturn },
        { "var",         TokenType::KwVar },
        { "val",         TokenType::KwVal },
        { "true",        TokenType::KwTrue },
        { "false",       TokenType::KwFalse },
        { "constructor", TokenType::KwConstructor },
        { "interface",   TokenType::KwInterface },
        { "impl",        TokenType::KwImpl },
        { "func",        TokenType::KwFunc },
        { "struct",      TokenType::KwStruct },
        { "module",      TokenType::KwModule },
        { "import",      TokenType::KwImport },
        { "operator",    TokenType::KwOperator },
        { "this",        TokenType::KwThis },
    };
}

bool Lexer::isAtEnd() const {
    return position >= source.size();
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[position];
}

char Lexer::peekNext() const {
    return (position + 1 >= source.size()) ? '\0' : source[position + 1];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    return source[position++];
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line++;
            advance();
        } else if (c == '/') {
            if (peekNext() == '/') {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (peekNext() == '*') {
                advance();
                advance();
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance();
                        advance();
                        break;
                    }
                    if (advance() == '\n') line++;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

Token Lexer::scanToken() {
    char c = advance();
    switch (c) {
    case '(': return {TokenType::LParen, "(", line};
    case ')': return {TokenType::RParen, ")", line};
    case '{': return {TokenType::LBrace, "{", line};
    case '}': return {TokenType::RBrace, "}", line};
    case '[': return {TokenType::LBracket, "[", line};
    case ']': return {TokenType::RBracket, "]", line};
    case ',': return {TokenType::Comma, ",", line};
    case ';': return {TokenType::Semicolon, ";", line};
    case '+': return {TokenType::Plus, "+", line};
    case '"': return stringToken();
    case '-': 
        if (peek() == '>') {
            advance();
            return {TokenType::ReturnType, "->", line};
        }
        return {TokenType::Minus, "-", line};
    case '*': return {TokenType::Star, "*", line};
    case '/': return {TokenType::Slash, "/", line};
    case '=': 
        if (peek() == '=') {
            advance();
            return {TokenType::Equal, "==", line};
        }
        return {TokenType::Assign, "=", line};
    case '!': 
        if (peek() == '=') {
            advance();
            return {TokenType::NotEqual, "!=", line};
        }
        return {TokenType::Not, "!", line};
    case '<': 
        if (peek() == '=') {
            advance();
            return {TokenType::LessEqual, "<=", line};
        }
        return {TokenType::Less, "<", line};
    case '>': 
        if (peek() == '=') {
            advance();
            return {TokenType::GreaterEqual, ">=", line};
        }
        return {TokenType::Greater, ">", line};
    case '&': 
        if (peek() == '&') {
            advance();
            return {TokenType::And, "&&", line};
        }
        break;
    case '|': 
        if (peek() == '|') {
            advance();
            return {TokenType::Or, "||", line};
        }
        break;
    case ':': return {TokenType::Colon, ":", line};
    case '.': return {TokenType::Dot, ".", line};
    case '~': return {TokenType::Tilde, "~", line};
    default:
        if (std::isalpha(c) || c == '_') {
            position--;
            return identifierToken();
        } 
        if (std::isdigit(c)) {
            position--;
            return numberToken();
        }
        return {TokenType::Error, std::string(1, c), line};
    }
    return {TokenType::Error, std::string(1, c), line};
}

Token Lexer::stringToken() {
    std::string value;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        value += advance();
    }
    if (isAtEnd()) {
        return {TokenType::Error, "Unterminated string", line};
    }
    advance();
    return {TokenType::Str, value, line};
}

Token Lexer::identifierToken() {
    std::string text;
    while (std::isalnum(peek()) || peek() == '_') {
        text += advance();
    }
    
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return {it->second, text, line};
    }
    return {TokenType::Identifier, text, line};
}

Token Lexer::numberToken() {
    std::string text;
    bool hasDot = false;
    bool isFloat = false;
    
    while (std::isdigit(peek()) || (!hasDot && peek() == '.')) {
        if (peek() == '.') {
            if (hasDot) break;
            hasDot = true;
            isFloat = true;
        }
        text += advance();
    }
    
    if (isFloat) {
        return {TokenType::Number, text, line};
    }
    return {TokenType::Number, text, line};
}
