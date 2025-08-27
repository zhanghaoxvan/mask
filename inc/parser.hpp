#pragma once

#include "inc.hpp"
#include "lexer.hpp"
#include "ast.hpp"

class ErrorReporter {
public:
    struct Error {
        int line;
        std::string message;
        std::string context;
    };
    
    Error report(int line, const std::string& message, const std::string& context = "");
    bool hasErrors() const;
    const std::vector<Error>& getErrors() const;
    void clear();
    
private:
    std::vector<Error> errors;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, ErrorReporter& errorReporter);
    ASTTree parse();
    
private:
    struct Scope {
        std::unordered_map<std::string, ASTNode*> symbols;
        std::string name;
    };
    
    void enterScope(const std::string& name = "");
    void exitScope();
    void declare(const std::string& name, ASTNode* node);
    ASTNode* resolve(const std::string& name);
    
    const Token& peek() const;
    const Token& advance();
    const Token& previous() const;
    bool isAtEnd() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const std::string& message);
    void sync();
    void error(const Token& token, const std::string& message);
    
private:
    const std::vector<Token> tokens;
    size_t current = 0;
    ErrorReporter& errorReporter;
    std::vector<Scope> scopes;
    std::string currentModule;
};
